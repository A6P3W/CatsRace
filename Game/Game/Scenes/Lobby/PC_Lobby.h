#pragma once

#include "PlayerController.h"
#include "NetworkTypes.h"

#include <vector>

class ALobbyPlayerState;
class ALobbyScene;
class WLobbyHUD;

class PC_Lobby : public APlayerController
{
public:
	DEFINE_ACTOR_CLASS(PC_Lobby)

	PC_Lobby();

	std::vector<ALobbyPlayerState*> GetPlayerStates();
	ALobbyPlayerState* FindLocalPlayerState();
	ALobbyPlayerState* FindHostPlayerState();
	ALobbyPlayerState* FindPlayerState(FNetworkConnectionId ConnectionId);
	int GetMaxPlayers() const;
	FNetworkSceneId GetSelectedGameSceneId() const;
	void ApplyHostLobbyOptions();
	void SetMaxPlayers(int InMaxPlayers);
	void StartGame();

protected:
	void BeginPlay() override;

private:
	ALobbyScene* GetLobbyScene() const;

	WLobbyHUD* m_LobbyHUD = nullptr;
};