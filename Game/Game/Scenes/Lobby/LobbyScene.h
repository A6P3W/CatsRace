#pragma once

#include "GameModeBase.h"
#include "NetworkTypes.h"

#include <vector>

class ALobbyPlayerState;
class APlayerController;

class ALobbyScene : public AGameModeBase
{
public:
	DEFINE_ACTOR_CLASS(ALobbyScene)

	ALobbyScene();
	void BeginPlay() override;
	void Draw() override;
	APlayerController* OnClientConnected(FNetworkConnectionId ConnectionId) override;
	void OnClientDisconnected(FNetworkConnectionId ConnectionId) override;

private:
	std::vector<ALobbyPlayerState*> GetPlayerStates();
	ALobbyPlayerState* FindLocalPlayerState();
	ALobbyPlayerState* FindHostPlayerState();
	ALobbyPlayerState* FindPlayerState(FNetworkConnectionId ConnectionId);
	ALobbyPlayerState* SpawnPlayerState(FNetworkConnectionId ConnectionId);
	void EnsureHostPlayerState();
	void SaveLobbyResultsToGameInstance(const std::vector<ALobbyPlayerState*>& States);
	void StartGame();

	int MaxPlayers = 4;
	FNetworkSceneId SelectedGameSceneId = 10;
};