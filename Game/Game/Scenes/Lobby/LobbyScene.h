#pragma once

#include <vector>

#include "GameModeBase.h"
#include "NetworkTypes.h"

class ALobbyPlayerState;
class APlayerController;
class PC_Lobby;

class ALobbyScene : public AGameModeBase {
  friend class PC_Lobby;

 public:
  DEFINE_ACTOR_CLASS(ALobbyScene)

  ALobbyScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;
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
  PC_Lobby* m_LobbyPlayerController = nullptr;
};