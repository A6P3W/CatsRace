#include "Scenes/Lobby/LobbyPlayerState.h"

#include <algorithm>
#include <utility>

#include "Core/MapData.h"

namespace {
enum : FNetworkRPCId {
  RPC_ServerSetPlayerName = 1,
  RPC_ServerSetReady = 2,
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
  RegisterReplicatedProperty(&bReady);
  RegisterReplicatedProperty(&SelectedLevelPath);
  RegisterReplicatedProperty(&MaxPlayers);
  RegisterReplicatedProperty(&bFinished);
  RegisterReplicatedProperty(&FinishTime);
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
  RegisterRPC(RPC_ServerSetReady, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyReady);
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

void ALobbyPlayerState::SetReady(bool bInReady) {
  if (bHasAuthority) {
    ApplyReady(bInReady);
    return;
  }
  InvokeRPC(RPC_ServerSetReady, ENetRPCType::Server, ENetPacketReliability::Reliable, bInReady);
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

void ALobbyPlayerState::ApplyReady(bool bInReady) {
  bReady = bInReady;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyLobbyOptions(std::string InSelectedLevelPath, int InMaxPlayers) {
  SelectedLevelPath =
      IsAvailableLevelPath(InSelectedLevelPath) ? std::move(InSelectedLevelPath) : GetDefaultLevelPath();
  MaxPlayers = InMaxPlayers < 1 ? 1 : InMaxPlayers;
  MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyFinishResult(bool bInFinished, float InFinishTime) {
  bFinished = bInFinished;
  FinishTime = InFinishTime;
  MarkReplicatedStateDirty();
}

