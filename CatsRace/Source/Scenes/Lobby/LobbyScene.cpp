#include "Scenes/Lobby/LobbyScene.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <unordered_set>

#include "ActorManager.h"
#include "Actors/HostServerTravelActor.h"
#include "Core/GI_main.h"
#include "Core/MapData.h"
#include "EOSLobbyManager.h"
#include "Log.h"
#include "NetworkManager.h"
#include "OnlinePlayManager.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/LobbyRoadActor.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "Services/LeaderBoardManager.h"
#include "World.h"
REGISTER_GAME_MODE(ALobbyScene)

namespace {
constexpr const char* LobbyStateAttributeKey = "LOBBY_STATE";
constexpr const char* LobbyStateWaiting = "WAITING";
constexpr const char* LobbyStateRacing = "RACING";

FLobbyAttribute MakeLobbyStateAttribute(const char* State) {
  return {LobbyStateAttributeKey, FLobbyAttributeValue::FromString(State), true};
}
}  // namespace

ALobbyScene::ALobbyScene() {
  SetUpdateableAnytime(true);
  SelectedLevelPath = AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
  SetDefaultPlayerControllerClass(PC_Lobby::StaticClassName());
}

void ALobbyScene::BeginPlay() {
  AGameModeBase::BeginPlay();

  GetWorld()->SpawnActor<ALobbyRoadActor>();

  AHostServerTravelActor::SpawnForListenServer(*GetWorld());

  if (GetWorld()->IsServer()) {
    EnsureHostPlayerState();
    if (OnlinePlayManager::GetInstance().IsInLobby()) {
      EOSLobbyManager::GetInstance().UpdateCurrentLobbyAttributes(
          {MakeLobbyStateAttribute(LobbyStateWaiting)}, [](bool bSuccess) { (void)bSuccess; }
      );
    }
  }
}

void ALobbyScene::OnUpdate(float DeltaTime) {
  if (!GetWorld()->IsServer() || bStartTravelRequested) {
    return;
  }

  if (RaceStartState == ERaceStartState::WaitingForReady) {
    GhostReadyTimeoutRemaining -= DeltaTime;
    if (GhostReadyTimeoutRemaining <= 0.0f) {
      M_LOG(Warning, "Ghost ready ACK timeout; continuing without all ACKs");
      BeginStartCountdown();
    }
    return;
  }
  if (RaceStartState != ERaceStartState::Countdown || StartCountdownRemaining < 0.0f) {
    return;
  }

  StartCountdownRemaining -= DeltaTime;
  const int seconds = (std::max)(0, static_cast<int>(std::ceil(StartCountdownRemaining)));
  if (seconds != LastPublishedCountdownSeconds) {
    LastPublishedCountdownSeconds = seconds;
    if (auto* hostState = FindHostPlayerState()) {
      hostState->SetStartCountdownSeconds(seconds);
    }
  }
  if (StartCountdownRemaining > 0.0f) {
    return;
  }
  bStartTravelRequested = true;
  SaveLobbyResultsToGameInstance(GetPlayerStates());
  GetWorld()->ServerTravel(PendingStartLevelPath);
}

void ALobbyScene::Draw() { AGameModeBase::Draw(); }

APlayerController* ALobbyScene::OnClientConnected(FNetworkConnectionId ConnectionId) {
  SpawnPlayerState(ConnectionId);
  return AGameModeBase::OnClientConnected(ConnectionId);
}

void ALobbyScene::OnClientDisconnected(FNetworkConnectionId ConnectionId) {
  if (auto* state = FindPlayerState(ConnectionId)) {
    state->Destroy();
  }
  AGameModeBase::OnClientDisconnected(ConnectionId);
  if (RaceStartState == ERaceStartState::WaitingForReady) {
    NotifyGhostReady(ConnectionId);
  }
}

std::vector<ALobbyPlayerState*> ALobbyScene::GetPlayerStates() {
  std::vector<ALobbyPlayerState*> states;
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return states;
  }
  for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    if (auto* state = dynamic_cast<ALobbyPlayerState*>(actorPtr.get())) {
      if (!state->IsPendingDestroy()) {
        states.push_back(state);
      }
    }
  }
  std::sort(
      states.begin(), states.end(), [](const ALobbyPlayerState* a, const ALobbyPlayerState* b) {
        return a->OwnerConnectionId < b->OwnerConnectionId;
      }
  );
  return states;
}

ALobbyPlayerState* ALobbyScene::FindLocalPlayerState() {
  const FNetworkConnectionId localId = NetworkManager::GetInstance().GetLocalConnectionId();
  for (auto* state : GetPlayerStates()) {
    if (!state) continue;
    if ((GetWorld()->IsServer() && state->OwnerConnectionId == 0) ||
        state->OwnerConnectionId == localId || state->bIsLocallyControlled) {
      return state;
    }
  }
  return nullptr;
}

ALobbyPlayerState* ALobbyScene::FindHostPlayerState() { return FindPlayerState(0); }

ALobbyPlayerState* ALobbyScene::FindPlayerState(FNetworkConnectionId ConnectionId) {
  for (auto* state : GetPlayerStates()) {
    if (state && state->OwnerConnectionId == ConnectionId) {
      return state;
    }
  }
  return nullptr;
}

ALobbyPlayerState* ALobbyScene::SpawnPlayerState(FNetworkConnectionId ConnectionId) {
  if (auto* existing = FindPlayerState(ConnectionId)) {
    return existing;
  }

  auto* state = GetWorld()->SpawnActor<ALobbyPlayerState>();
  state->OwnerConnectionId = ConnectionId;
  state->bReplicates = true;
  state->bHasAuthority = true;
  state->bIsLocallyControlled = ConnectionId == 0;
  state->SetPlayerColorIndex(AllocatePlayerColorIndex());

  std::string defaultName = "Player " + std::to_string(ConnectionId == 0 ? 1 : ConnectionId + 1);
  if (ConnectionId == 0) {
    if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
      if (!gi->player_name.empty()) {
        defaultName = gi->player_name;
      }
      const bool bSavedLevelPathAvailable =
          std::any_of(AvailableMaps.begin(), AvailableMaps.end(), [gi](const FMapInfo& Map) {
            return Map.LevelPath == gi->last_level_path;
          });
      if (bSavedLevelPathAvailable) {
        SelectedLevelPath = gi->last_level_path;
      }
    }
  }
  state->SetPlayerName(defaultName);
  state->SetLobbyOptions(SelectedLevelPath, MaxPlayers);
  return state;
}

uint8_t ALobbyScene::AllocatePlayerColorIndex() {
  std::array<bool, PlayerColorPalette.size()> usedColors{};
  for (const auto* state : GetPlayerStates()) {
    if (!state) {
      continue;
    }

    const uint8_t colorIndex = state->GetPlayerColorIndex();
    if (colorIndex < usedColors.size()) {
      usedColors[colorIndex] = true;
    }
  }

  for (size_t offset = 0; offset < PlayerColorPalette.size(); ++offset) {
    const uint8_t colorIndex =
        static_cast<uint8_t>((NextPlayerColorIndex + offset) % PlayerColorPalette.size());
    if (!usedColors[colorIndex]) {
      NextPlayerColorIndex = static_cast<uint8_t>((colorIndex + 1) % PlayerColorPalette.size());
      return colorIndex;
    }
  }

  return InvalidPlayerColorIndex;
}

void ALobbyScene::EnsureHostPlayerState() { SpawnPlayerState(0); }

void ALobbyScene::SaveLobbyResultsToGameInstance(const std::vector<ALobbyPlayerState*>& States) {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    gi->last_level_path = SelectedLevelPath;
    if (const FMapInfo* Map = FindMapInfo(SelectedLevelPath)) {
      gi->RaceMapId = Map->MapId;
      gi->RaceMapVersion = Map->MapVersion;
    }
    gi->multiplayer_results.clear();
    for (const auto* state : States) {
      if (!state) continue;
      GI_main::FMultiplayerResult result;
      result.ConnectionId = state->OwnerConnectionId;
      result.PlayerName = state->GetPlayerName();
      result.PlayerColorIndex = state->GetPlayerColorIndex();
      result.IdType = gi->BoothMode ? "PlayerName" : "DeviceId";
      result.UserId = gi->BoothMode ? result.PlayerName : state->GetDeviceId();
      result.IdentityKey = result.IdType + ":" + result.UserId;
      result.bFinished = false;
      result.FinishTime = 0.0f;
      gi->multiplayer_results.push_back(result);
    }
  }
}

bool ALobbyScene::ArePlayerIdentitiesReady(const std::vector<ALobbyPlayerState*>& States) const {
  const auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance || States.empty()) {
    return false;
  }

  std::unordered_set<std::string> IdentityKeys;
  for (const ALobbyPlayerState* State : States) {
    if (!State) {
      return false;
    }
    const std::string UserId =
        GameInstance->BoothMode ? State->GetPlayerName() : State->GetDeviceId();
    const std::string IdType = GameInstance->BoothMode ? "PlayerName" : "DeviceId";
    if (UserId.empty() || !IdentityKeys.insert(IdType + ":" + UserId).second) {
      return false;
    }
  }
  return true;
}

void ALobbyScene::FetchRaceGhosts() {
  const FMapInfo* Map = FindMapInfo(PendingStartLevelPath);
  if (!Map || Map->MapId.empty()) {
    M_LOG(Error, "Cannot fetch ghosts: selected map metadata is invalid");
    RaceStartState = ERaceStartState::Idle;
    return;
  }

  RaceStartState = ERaceStartState::FetchingGhosts;
  auto* Manager = GetWorld()->SpawnActor<LeaderBoardManager>();
  Manager->FetchRaceGhosts(
      Map->MapId,
      Map->MapVersion,
      [this](bool bSuccess, const std::vector<FRaceGhostData>& Ghosts) {
        if (auto* GameInstance =
                dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
          GameInstance->RaceGhosts = bSuccess ? Ghosts : std::vector<FRaceGhostData>{};
        }
        if (!bSuccess) {
          M_LOG(Warning, "Ghost fetch failed; continuing with an empty ghost set");
        }
        DistributeRaceGhosts();
      }
  );
}

void ALobbyScene::DistributeRaceGhosts() {
  const auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  const std::vector<FRaceGhostData> EmptyGhosts;
  const auto& Ghosts = GameInstance ? GameInstance->RaceGhosts : EmptyGhosts;

  GhostReadyConnections.clear();
  RaceStartState = ERaceStartState::WaitingForReady;
  GhostReadyTimeoutRemaining = 5.0f;
  for (ALobbyPlayerState* State : GetPlayerStates()) {
    if (State) {
      State->SendRaceGhosts(Ghosts);
    }
  }
}

void ALobbyScene::NotifyGhostReady(FNetworkConnectionId ConnectionId) {
  if (!GetWorld()->IsServer() || RaceStartState != ERaceStartState::WaitingForReady) {
    return;
  }
  GhostReadyConnections.insert(ConnectionId);
  const auto States = GetPlayerStates();
  const bool bAllReady = std::all_of(States.begin(), States.end(), [this](const auto* State) {
    return State && GhostReadyConnections.contains(State->OwnerConnectionId);
  });
  if (bAllReady) {
    BeginStartCountdown();
  }
}

void ALobbyScene::BeginStartCountdown() {
  RaceStartState = ERaceStartState::Countdown;
  GhostReadyTimeoutRemaining = -1.0f;
  StartCountdownRemaining = 3.0f;
  LastPublishedCountdownSeconds = 3;
  if (auto* HostState = FindHostPlayerState()) {
    HostState->SetStartCountdownSeconds(3);
  }
}

void ALobbyScene::StartGame() {
  const auto States = GetPlayerStates();
  if (!GetWorld()->IsServer() || RaceStartState != ERaceStartState::Idle || bStartTravelRequested ||
      SelectedLevelPath.empty() || GetPlayerStates().empty()) {
    return;
  }
  if (!ArePlayerIdentitiesReady(States)) {
    M_LOG(Warning, "Cannot start race until all player identities are unique and ready");
    return;
  }
  PendingStartLevelPath = SelectedLevelPath;
  SaveLobbyResultsToGameInstance(States);
  FetchRaceGhosts();
  if (OnlinePlayManager::GetInstance().IsInLobby()) {
    EOSLobbyManager::GetInstance().UpdateCurrentLobbyAttributes(
        {MakeLobbyStateAttribute(LobbyStateRacing)}, [](bool bSuccess) { (void)bSuccess; }
    );
  }
}
