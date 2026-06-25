#include "Scenes/Lobby/LobbyPlayerState.h"

#include "Core/GameSceneIds.h"
#include "NetBuffer.h"

namespace
{
	enum : FNetworkRPCId
	{
		RPC_ServerSetPlayerName = 1,
		RPC_ServerSetReady = 2,
		RPC_ServerSetLobbyOptions = 3,
		RPC_ServerSetFinishResult = 4
	};
}

REGISTER_ACTOR(ALobbyPlayerState)

ALobbyPlayerState::ALobbyPlayerState()
{
	bReplicates = true;
	SelectedGameSceneId = GameSceneIds::Game01;
	InitializeRPCs();
}

ALobbyPlayerState::ALobbyPlayerState(const FVector2D& Location, FRotator Rotation)
	: ALobbyPlayerState()
{
	SetActorLocation(Location);
	SetActorRotation(Rotation);
}

void ALobbyPlayerState::InitializeRPCs()
{
	RegisterRPC(RPC_ServerSetPlayerName, ENetRPCType::Server, [this](FNetBuffer& Payload) {
		std::string name;
		if (Payload.ReadString(name)) {
			ApplyPlayerName(name);
		}
	});
	RegisterRPC(RPC_ServerSetReady, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyReady);
	RegisterRPC(RPC_ServerSetLobbyOptions, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyLobbyOptions);
	RegisterRPC(RPC_ServerSetFinishResult, ENetRPCType::Server, this, &ALobbyPlayerState::ApplyFinishResult);
}

void ALobbyPlayerState::SetPlayerName(const std::string& Name)
{
	if (bHasAuthority) {
		ApplyPlayerName(Name);
		return;
	}

	FNetBuffer payload;
	payload.WriteString(Name);
	InvokeRPCWithPayload(RPC_ServerSetPlayerName, ENetRPCType::Server, ENetPacketReliability::Reliable, payload);
}

void ALobbyPlayerState::SetReady(bool bInReady)
{
	if (bHasAuthority) {
		ApplyReady(bInReady);
		return;
	}
	InvokeRPC(RPC_ServerSetReady, ENetRPCType::Server, ENetPacketReliability::Reliable, bInReady);
}

void ALobbyPlayerState::SetLobbyOptions(FNetworkSceneId InSelectedGameSceneId, int InMaxPlayers)
{
	if (bHasAuthority) {
		ApplyLobbyOptions(InSelectedGameSceneId, InMaxPlayers);
		return;
	}
	InvokeRPC(RPC_ServerSetLobbyOptions, ENetRPCType::Server, ENetPacketReliability::Reliable, InSelectedGameSceneId, InMaxPlayers);
}

void ALobbyPlayerState::SetFinishResult(bool bInFinished, float InFinishTime)
{
	if (bHasAuthority) {
		ApplyFinishResult(bInFinished, InFinishTime);
		return;
	}
	InvokeRPC(RPC_ServerSetFinishResult, ENetRPCType::Server, ENetPacketReliability::Reliable, bInFinished, InFinishTime);
}

void ALobbyPlayerState::ApplyPlayerName(const std::string& Name)
{
	PlayerName = Name.empty() ? "Player" : Name;
	MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyReady(bool bInReady)
{
	bReady = bInReady;
	MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyLobbyOptions(FNetworkSceneId InSelectedGameSceneId, int InMaxPlayers)
{
	SelectedGameSceneId = InSelectedGameSceneId == 0 ? GameSceneIds::Game01 : InSelectedGameSceneId;
	MaxPlayers = InMaxPlayers < 1 ? 1 : InMaxPlayers;
	MarkReplicatedStateDirty();
}

void ALobbyPlayerState::ApplyFinishResult(bool bInFinished, float InFinishTime)
{
	bFinished = bInFinished;
	FinishTime = InFinishTime;
	MarkReplicatedStateDirty();
}

void ALobbyPlayerState::SerializeNetworkState(FNetBuffer& OutBuffer)
{
	AActor::SerializeNetworkState(OutBuffer);
	OutBuffer.WriteString(PlayerName);
	OutBuffer.Write(bReady);
	OutBuffer.Write(SelectedGameSceneId);
	OutBuffer.Write(MaxPlayers);
	OutBuffer.Write(bFinished);
	OutBuffer.Write(FinishTime);
}

bool ALobbyPlayerState::DeserializeNetworkState(FNetBuffer& InBuffer)
{
	if (!AActor::DeserializeNetworkState(InBuffer)) return false;
	if (!InBuffer.ReadString(PlayerName)) return false;
	if (!InBuffer.Read(bReady)) return false;
	if (!InBuffer.Read(SelectedGameSceneId)) return false;
	if (!InBuffer.Read(MaxPlayers)) return false;
	if (!InBuffer.Read(bFinished)) return false;
	if (!InBuffer.Read(FinishTime)) return false;
	return true;
}

void ALobbyPlayerState::SerializeNetworkSpawn(FNetBuffer& OutBuffer)
{
	OutBuffer.WriteString(GetActorClassName());
	SerializeNetworkState(OutBuffer);
}

bool ALobbyPlayerState::DeserializeNetworkSpawn(FNetBuffer& InBuffer)
{
	std::string className;
	if (!InBuffer.ReadString(className)) return false;
	return DeserializeNetworkState(InBuffer);
}