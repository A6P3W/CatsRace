#include "Scenes/Lobby/LobbyScene.h"

#include <algorithm>
#include <cmath>

#include "Core/GI_main.h"
#include "Core/MapData.h"
#include "NetworkManager.h"
#include "OnlineSessionManager.h"
#include "ActorManager.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "World.h"
#include <filesystem>
REGISTER_GAME_MODE(ALobbyScene)

namespace {
constexpr const char* LobbyStateAttributeKey = "LOBBY_STATE";
constexpr const char* LobbyStateWaiting = "WAITING";
constexpr const char* LobbyStateRacing = "RACING";

FLobbyAttribute MakeLobbyStateAttribute(const char* State) {
  return {LobbyStateAttributeKey, FLobbyAttributeValue::FromString(State), true};
}
}

ALobbyScene::ALobbyScene() {
  SetUpdateableAnytime(true);
  SelectedLevelPath = AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
  SetDefaultPlayerControllerClass(PC_Lobby::StaticClassName());
}

void ALobbyScene::BeginPlay() {
  AGameModeBase::BeginPlay();


  if (GetWorld()->IsServer()) {
    EnsureHostPlayerState();
    if (OnlineSessionManager::Get().IsInLobby()) {
      OnlineSessionManager::Get().UpdateCurrentLobbyAttributes(
          {MakeLobbyStateAttribute(LobbyStateWaiting)}, [](bool bSuccess) { (void)bSuccess; }
      );
    }
  }
}  

void ALobbyScene::OnUpdate(float DeltaTime) {
  if (!GetWorld()->IsServer() || StartCountdownRemaining < 0.0f || bStartTravelRequested) {
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

  std::string defaultName = "Player " + std::to_string(ConnectionId == 0 ? 1 : ConnectionId + 1);
  if (ConnectionId == 0) {
    if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
      if (!gi->player_name.empty()) {
        defaultName = gi->player_name;
      }
      const bool bSavedLevelPathAvailable = std::any_of(
          AvailableMaps.begin(),
          AvailableMaps.end(),
          [gi](const FMapInfo& Map) { return Map.LevelPath == gi->last_level_path; }
      );
      if (bSavedLevelPathAvailable) {
        SelectedLevelPath = gi->last_level_path;
      }
    }
  }
  state->SetPlayerName(defaultName);
  state->SetLobbyOptions(SelectedLevelPath, MaxPlayers);
  return state;
}

void ALobbyScene::EnsureHostPlayerState() { SpawnPlayerState(0); }

void ALobbyScene::SaveLobbyResultsToGameInstance(const std::vector<ALobbyPlayerState*>& States) {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    gi->last_level_path = SelectedLevelPath;
    gi->multiplayer_results.clear();
    for (const auto* state : States) {
      if (!state) continue;
      GI_main::FMultiplayerResult result;
      result.ConnectionId = state->OwnerConnectionId;
      result.PlayerName = state->GetPlayerName();
      result.bFinished = false;
      result.FinishTime = 0.0f;
      gi->multiplayer_results.push_back(result);
    }
  }
}

void ALobbyScene::StartGame() {
  if (!GetWorld()->IsServer() || StartCountdownRemaining >= 0.0f || bStartTravelRequested ||
      SelectedLevelPath.empty() || GetPlayerStates().empty()) {
    return;
  }
  PendingStartLevelPath = SelectedLevelPath;
  StartCountdownRemaining = 5.0f;
  LastPublishedCountdownSeconds = 5;
  if (auto* hostState = FindHostPlayerState()) {
    hostState->SetStartCountdownSeconds(5);
  }
  if (OnlineSessionManager::Get().IsInLobby()) {
    OnlineSessionManager::Get().UpdateCurrentLobbyAttributes(
        {MakeLobbyStateAttribute(LobbyStateRacing)}, [](bool bSuccess) { (void)bSuccess; }
    );
  }
}

