#include "Scenes/Lobby/PC_Lobby.h"

#include "Core/GameSceneIds.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Lobby/UI/WLobbyHUD.h"
#include "UIManager.h"
#include "World.h"

#include <algorithm>

REGISTER_ACTOR(PC_Lobby)

PC_Lobby::PC_Lobby()
{
	SetUpdateableAnytime(true);
}

void PC_Lobby::BeginPlay()
{
	APlayerController::BeginPlay();
	SetInputMode(EInputMode::UIOnly);
	SetupInputMappings();

	m_LobbyHUD = GetWorld()->SpawnActor<WLobbyHUD>();
	m_LobbyHUD->SetLobbyController(this);
	UIManager::GetInstance()->AddWidget(m_LobbyHUD);
}

std::vector<ALobbyPlayerState*> PC_Lobby::GetPlayerStates()
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

ALobbyPlayerState* PC_Lobby::FindLocalPlayerState()
{
	const FNetworkConnectionId localId = NetworkManager::GetInstance().GetLocalConnectionId();
	for (auto* state : GetPlayerStates()) {
		if (!state) {
			continue;
		}
		if ((GetWorld()->IsServer() && state->OwnerConnectionId == 0) || state->OwnerConnectionId == localId || state->bIsLocallyControlled) {
			return state;
		}
	}
	return nullptr;
}

ALobbyPlayerState* PC_Lobby::FindHostPlayerState()
{
	return FindPlayerState(0);
}

ALobbyPlayerState* PC_Lobby::FindPlayerState(FNetworkConnectionId ConnectionId)
{
	for (auto* state : GetPlayerStates()) {
		if (state && state->OwnerConnectionId == ConnectionId) {
			return state;
		}
	}
	return nullptr;
}

int PC_Lobby::GetMaxPlayers() const
{
	if (auto* lobbyScene = GetLobbyScene()) {
		return lobbyScene->MaxPlayers;
	}
	if (auto* hostState = const_cast<PC_Lobby*>(this)->FindHostPlayerState()) {
		return hostState->GetMaxPlayers();
	}
	return 0;
}

FNetworkSceneId PC_Lobby::GetSelectedGameSceneId() const
{
	if (auto* lobbyScene = GetLobbyScene()) {
		return lobbyScene->SelectedGameSceneId;
	}
	if (auto* hostState = const_cast<PC_Lobby*>(this)->FindHostPlayerState()) {
		return hostState->GetSelectedGameSceneId();
	}
	return GameSceneIds::Game01;
}

void PC_Lobby::ApplyHostLobbyOptions()
{
	auto* lobbyScene = GetLobbyScene();
	auto* hostState = FindHostPlayerState();
	if (!lobbyScene || !hostState) {
		return;
	}

	lobbyScene->SelectedGameSceneId = hostState->GetSelectedGameSceneId();
	lobbyScene->MaxPlayers = hostState->GetMaxPlayers();
}

void PC_Lobby::SetMaxPlayers(int InMaxPlayers)
{
	auto* lobbyScene = GetLobbyScene();
	if (!lobbyScene) {
		return;
	}

	lobbyScene->MaxPlayers = InMaxPlayers;
	if (auto* host = lobbyScene->FindHostPlayerState()) {
		host->SetLobbyOptions(lobbyScene->SelectedGameSceneId, lobbyScene->MaxPlayers);
	}
}

void PC_Lobby::StartGame()
{
	if (auto* lobbyScene = GetLobbyScene()) {
		lobbyScene->StartGame();
	}
}

ALobbyScene* PC_Lobby::GetLobbyScene() const
{
	auto* self = const_cast<PC_Lobby*>(this);
	if (!self->GetWorld()) {
		return nullptr;
	}
	return dynamic_cast<ALobbyScene*>(self->GetWorld()->GetGameMode());
}