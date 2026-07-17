#pragma once

#include <string>
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

  float StartCountdownRemaining = -1.0f;
  int LastPublishedCountdownSeconds = -1;
  bool bStartTravelRequested = false;
  std::string PendingStartLevelPath;

  int MaxPlayers = 4;
  std::string SelectedLevelPath;
  PC_Lobby* m_LobbyPlayerController = nullptr;
};
