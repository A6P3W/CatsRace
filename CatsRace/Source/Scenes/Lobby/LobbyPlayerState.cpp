#include "Scenes/Lobby/LobbyPlayerState.h"

#include <algorithm>
#include <utility>

#include "Core/MapData.h"

namespace {
enum : FNetworkRPCId {
  RPC_ServerSetPlayerName = 1,
  RPC_ServerSetLobbyOptions = 3,
  RPC_ServerSetFinishResult = 4
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
  RegisterReplicatedProperty(
      &SelectedLevelPath, this, &ALobbyPlayerState::OnRepSelectedLevelPath
  );
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
}

void ALobbyPlayerState::SetPlayerName(const std::string& Name) {
  if (bHasAuthority) {
    ApplyPlayerName(Name);
    return;
  }

  InvokeRPC(RPC_ServerSetPlayerName, ENetRPCType::Server, ENetPacketReliability::Reliable, Name);
}

void ALobbyPlayerState::SetLobbyOptions(
    const std::string& InSelectedLevelPath, int InMaxPlayers
) {
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
