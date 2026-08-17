#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Core/PlayerColorPalette.h"
#include "GameModeBase.h"
#include "NetworkTypes.h"
#include "SoundComponent.h"

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
  void NotifyGhostReady(FNetworkConnectionId ConnectionId);

 private:
  enum class ERaceStartState { Idle, FetchingGhosts, WaitingForReady, Countdown };

  std::vector<ALobbyPlayerState*> GetPlayerStates();
  ALobbyPlayerState* FindLocalPlayerState();
  ALobbyPlayerState* FindHostPlayerState();
  ALobbyPlayerState* FindPlayerState(FNetworkConnectionId ConnectionId);
  ALobbyPlayerState* SpawnPlayerState(FNetworkConnectionId ConnectionId);
  uint8_t AllocatePlayerColorIndex();
  void EnsureHostPlayerState();
  void SaveLobbyResultsToGameInstance(const std::vector<ALobbyPlayerState*>& States);
  bool ArePlayerIdentitiesReady(const std::vector<ALobbyPlayerState*>& States) const;
  void FetchRaceGhosts();
  void DistributeRaceGhosts();
  void BeginStartCountdown();
  void StartGame();
  MSoundComponent* m_bgmSound = nullptr;
  float StartCountdownRemaining = -1.0f;
  int LastPublishedCountdownSeconds = -1;
  bool bStartTravelRequested = false;
  std::string PendingStartLevelPath;
  ERaceStartState RaceStartState = ERaceStartState::Idle;
  float GhostReadyTimeoutRemaining = -1.0f;
  std::unordered_set<FNetworkConnectionId> GhostReadyConnections;

  int MaxPlayers = 4;
  uint8_t NextPlayerColorIndex = 0;
  std::string SelectedLevelPath;
  PC_Lobby* m_LobbyPlayerController = nullptr;
};
