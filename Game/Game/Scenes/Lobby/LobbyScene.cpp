#include "Scenes/Lobby/LobbyScene.h"

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"
#include "ObjectManager.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "World.h"

#include <algorithm>

REGISTER_GAME_MODE(ALobbyScene)

ALobbyScene::ALobbyScene()
{
	SetUpdateableAnytime(true);
	SelectedGameSceneId = GameSceneIds::Game01;
	DefaultPlayerControllerClass = PC_Lobby::StaticClassName();
}

void ALobbyScene::BeginPlay()
{
	AGameModeBase::BeginPlay();
	if (GetWorld()->IsServer()) {
		EnsureHostPlayerState();
	}

}

void ALobbyScene::OnUpdate(float DeltaTime)
{
	(void)DeltaTime;
}

void ALobbyScene::Draw()
{
	AGameModeBase::Draw();
}

APlayerController* ALobbyScene::OnClientConnected(FNetworkConnectionId ConnectionId)
{
	SpawnPlayerState(ConnectionId);
	return AGameModeBase::OnClientConnected(ConnectionId);
}

void ALobbyScene::OnClientDisconnected(FNetworkConnectionId ConnectionId)
{
	if (auto* state = FindPlayerState(ConnectionId)) {
		state->Destroy();
	}
}

std::vector<ALobbyPlayerState*> ALobbyScene::GetPlayerStates()
{
	std::vector<ALobbyPlayerState*> states;
	if (!GetWorld() || !GetWorld()->GetObjectManager()) {
		return states;
	}
	for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
		if (auto* state = dynamic_cast<ALobbyPlayerState*>(actorPtr.get())) {
			if (!state->IsPendingDestroy()) {
				states.push_back(state);
			}
		}
	}
	std::sort(states.begin(), states.end(), [](const ALobbyPlayerState* a, const ALobbyPlayerState* b) {
		return a->OwnerConnectionId < b->OwnerConnectionId;
	});
	return states;
}

ALobbyPlayerState* ALobbyScene::FindLocalPlayerState()
{
	const FNetworkConnectionId localId = NetworkManager::GetInstance().GetLocalConnectionId();
	for (auto* state : GetPlayerStates()) {
		if (!state) continue;
		if ((GetWorld()->IsServer() && state->OwnerConnectionId == 0) || state->OwnerConnectionId == localId || state->bIsLocallyControlled) {
			return state;
		}
	}
	return nullptr;
}

ALobbyPlayerState* ALobbyScene::FindHostPlayerState()
{
	return FindPlayerState(0);
}

ALobbyPlayerState* ALobbyScene::FindPlayerState(FNetworkConnectionId ConnectionId)
{
	for (auto* state : GetPlayerStates()) {
		if (state && state->OwnerConnectionId == ConnectionId) {
			return state;
		}
	}
	return nullptr;
}

ALobbyPlayerState* ALobbyScene::SpawnPlayerState(FNetworkConnectionId ConnectionId)
{
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
		}
	}
	state->SetPlayerName(defaultName);
	state->SetLobbyOptions(SelectedGameSceneId, MaxPlayers);
	return state;
}

void ALobbyScene::EnsureHostPlayerState()
{
	SpawnPlayerState(0);
}

void ALobbyScene::SaveLobbyResultsToGameInstance(const std::vector<ALobbyPlayerState*>& States)
{
	if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
		gi->last_game_scene_id = SelectedGameSceneId;
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

void ALobbyScene::StartGame()
{
	if (!GetWorld()->IsServer()) {
		return;
	}
	const auto states = GetPlayerStates();
	SaveLobbyResultsToGameInstance(states);
	GetWorld()->ServerTravel(SelectedGameSceneId == 0 ? GameSceneIds::Game01 : SelectedGameSceneId);
}