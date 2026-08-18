#include "Scenes/Lobby/LobbyPlayerState.h"

#include <algorithm>

#include "Core/GI_main.h"
#include "Core/MapData.h"
#include "Log.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Services/LeaderBoardManager.h"
#include "World.h"

namespace {
enum : FNetworkRPCId {
  RPC_ServerSetPlayerName = 1,
  RPC_ServerSetLobbyOptions = 3,
  RPC_ServerSetFinishResult = 4,
  RPC_ServerSetDeviceId = 5,
  RPC_ClientDownloadRaceGhosts = 6,
  RPC_ServerAcknowledgeRaceGhosts = 8
};

std::string GetDefaultLevelPath() {
  return AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
}

bool IsAvailableLevelPath(const std::string& LevelPath) {
  return std::any_of(AvailableMaps.begin(), AvailableMaps.end(), [&LevelPath](const FMapInfo& Map) {
    return Map.LevelPath == LevelPath;
  });
}
}  // namespace

REGISTER_ACTOR(ALobbyPlayerState)

ALobbyPlayerState::ALobbyPlayerState() {
  bReplicates = true;
  SelectedLevelPath = GetDefaultLevelPath();
  RegisterReplicatedProperty(&PlayerName);
  RegisterReplicatedProperty(&DeviceId);
  RegisterReplicatedProperty(&PlayerColorIndex);
  RegisterReplicatedProperty(&SelectedLevelPath, this, &ALobbyPlayerState::OnRepSelectedLevelPath);
  RegisterReplicatedProperty(&MaxPlayers);
  RegisterReplicatedProperty(&bFinished);
  RegisterReplicatedProperty(&FinishTime);
  RegisterReplicatedProperty(&StartCountdownSeconds);
  InitializeRPCs();
}

ALobbyPlayerState::ALobbyPlayerState(const FVector2D& Location, FRotator Rotation)
    : ALobbyPlayerState() {
  SetActorLocation(Location);
  SetActorRotation(Rotation);
}

void ALobbyPlayerState::InitializeRPCs() {
  RegisterRPC(
      RPC_ServerSetPlayerName, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyPlayerName
  );
  RegisterRPC(
      RPC_ServerSetLobbyOptions, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyLobbyOptions
  );
  RegisterRPC(
      RPC_ServerSetFinishResult, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyFinishResult
  );
  RegisterRPC(RPC_ServerSetDeviceId, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyDeviceId);
  RegisterRPC(
      RPC_ClientDownloadRaceGhosts,
      ENetRPCType::Client,
      this,
      &ALobbyPlayerState::ClientDownloadRaceGhosts
  );
  RegisterRPC(
      RPC_ServerAcknowledgeRaceGhosts,
      ENetRPCType::Server,
      this,
      &ALobbyPlayerState::ServerAcknowledgeRaceGhosts
  );
}

bool ALobbyPlayerState::SetPlayerName(const std::string& Name) {
  if (bHasAuthority) {
    ApplyPlayerName(Name);
    return true;
  }

  return InvokeRPC(
      RPC_ServerSetPlayerName, ENetRPCType::Server, ENetPacketReliability::Reliable, Name
  );
}

bool ALobbyPlayerState::SetDeviceId(const std::string& InDeviceId) {
  if (InDeviceId.empty()) {
    return false;
  }
  if (bHasAuthority) {
    ApplyDeviceId(InDeviceId);
    return true;
  }
  return InvokeRPC(
      RPC_ServerSetDeviceId, ENetRPCType::Server, ENetPacketReliability::Reliable, InDeviceId
  );
}

void ALobbyPlayerState::SetPlayerColorIndex(uint8_t InColorIndex) {
  if (PlayerColorIndex == InColorIndex) {
    return;
  }

  PlayerColorIndex = InColorIndex;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::SetLobbyOptions(const std::string& InSelectedLevelPath, int InMaxPlayers) {
  if (bHasAuthority) {
    ApplyLobbyOptions(InSelectedLevelPath, InMaxPlayers);
    return;
  }
  InvokeRPC(
      RPC_ServerSetLobbyOptions,
      ENetRPCType::Server,
      ENetPacketReliability::Reliable,
      InSelectedLevelPath,
      InMaxPlayers
  );
}

void ALobbyPlayerState::SetFinishResult(bool bInFinished, float InFinishTime) {
  if (bHasAuthority) {
    ApplyFinishResult(bInFinished, InFinishTime);
    return;
  }
  InvokeRPC(
      RPC_ServerSetFinishResult,
      ENetRPCType::Server,
      ENetPacketReliability::Reliable,
      bInFinished,
      InFinishTime
  );
}

void ALobbyPlayerState::ApplyPlayerName(const std::string& Name) {
  PlayerName = Name.empty() ? "Player" : Name;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyDeviceId(const std::string& InDeviceId) {
  if (InDeviceId.empty() || DeviceId == InDeviceId) {
    return;
  }
  DeviceId = InDeviceId;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyLobbyOptions(std::string InSelectedLevelPath, int InMaxPlayers) {
  SetSelectedMap(
      IsAvailableLevelPath(InSelectedLevelPath) ? InSelectedLevelPath : GetDefaultLevelPath()
  );
  MaxPlayers = InMaxPlayers < 1 ? 1 : InMaxPlayers;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyFinishResult(bool bInFinished, float InFinishTime) {
  bFinished = bInFinished;
  FinishTime = InFinishTime;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::SetStartCountdownSeconds(int InStartCountdownSeconds) {
  if (StartCountdownSeconds == InStartCountdownSeconds) {
    return;
  }

  StartCountdownSeconds = InStartCountdownSeconds;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::RequestRaceGhostDownload(const std::string& MapId, int MapVersion) {
  if (!bHasAuthority) {
    return;
  }
  InvokeRPC(
      RPC_ClientDownloadRaceGhosts,
      ENetRPCType::Client,
      ENetPacketReliability::Reliable,
      MapId,
      MapVersion
  );
}

void ALobbyPlayerState::ClientDownloadRaceGhosts(std::string MapId, int MapVersion) {
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance) {
    CompleteRaceGhostDownload();
    return;
  }
  GameInstance->RaceGhosts.clear();

  auto* Manager = GetWorld()->SpawnActor<LeaderBoardManager>();
  if (!Manager) {
    M_LOG(
        Warning, "Could not create the ghost download manager; continuing with an empty ghost set"
    );
    CompleteRaceGhostDownload();
    return;
  }
  Manager->FetchRaceGhosts(
      MapId,
      MapVersion,
      [this, GameInstance](bool bSuccess, const std::vector<FRaceGhostData>& Ghosts) {
        GameInstance->RaceGhosts = bSuccess ? Ghosts : std::vector<FRaceGhostData>{};
        if (!bSuccess) {
          M_LOG(Warning, "Ghost fetch failed; continuing with an empty ghost set");
        }
        CompleteRaceGhostDownload();
      }
  );
}

void ALobbyPlayerState::CompleteRaceGhostDownload() {
  InvokeRPC(RPC_ServerAcknowledgeRaceGhosts, ENetRPCType::Server, ENetPacketReliability::Reliable);
}

void ALobbyPlayerState::ServerAcknowledgeRaceGhosts() {
  if (auto* LobbyScene = dynamic_cast<ALobbyScene*>(GetWorld()->GetGameMode())) {
    LobbyScene->NotifyGhostReady(OwnerConnectionId);
  }
}

ALobbyPlayerState::FCallbackHandle ALobbyPlayerState::AddOnSelectedMapChanged(
    FSelectedMapChangedCallback Callback
) {
  if (!Callback) {
    return 0;
  }

  const FCallbackHandle handle = NextSelectedMapChangedHandle++;
  SelectedMapChangedCallbacks.emplace(handle, std::move(Callback));
  return handle;
}

void ALobbyPlayerState::RemoveOnSelectedMapChanged(FCallbackHandle Handle) {
  if (Handle != 0) {
    SelectedMapChangedCallbacks.erase(Handle);
  }
}

void ALobbyPlayerState::SetSelectedMap(const std::string& NewLevelPath) {
  if (SelectedLevelPath == NewLevelPath) {
    return;
  }

  SelectedLevelPath = NewLevelPath;
  BroadcastOnSelectedMapChanged();
}

void ALobbyPlayerState::OnRepSelectedLevelPath(std::string OldLevelPath) {
  (void)OldLevelPath;
  BroadcastOnSelectedMapChanged();
}

void ALobbyPlayerState::BroadcastOnSelectedMapChanged() {
  const auto callbacks = SelectedMapChangedCallbacks;
  for (const auto& [handle, callback] : callbacks) {
    (void)handle;
    if (callback) {
      callback(SelectedLevelPath);
    }
  }
}
