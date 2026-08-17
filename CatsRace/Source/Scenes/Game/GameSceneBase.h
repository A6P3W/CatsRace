#pragma once
#include <TimerHandle.h>
#include <TimerManager.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "GameModeBase.h"
#include "UMath.h"

class WCountDown;
class WMainHUD;
class WPauseMenu;
class MGhostRecorderComponent;
class APlayer;
class APlayerController;

class AGameSceneBase : public AGameModeBase {
 public:
  AGameSceneBase();
  void OnUpdate(float DeltaTime) override;
  APlayerController* OnClientConnected(FNetworkConnectionId ConnectionId) override;
  void OnClientDisconnected(FNetworkConnectionId ConnectionId) override;
  void BeginPlay() override;
  void OnPlayerSpawned(
      APlayerController* Controller, APawn* Pawn, FNetworkConnectionId ConnectionId
  ) override;
  void SpawnHeldSpeedItemWithRespawn(FVector2D location);
  float GetRaceTime() const { return RaceTime; }
  const std::string& GetMapId() const { return MapId; }
  int GetMapVersion() const { return MapVersion; }
  void RaceFinish();
  void NotifyPlayerFinished(APlayer* Player);
  virtual void RestartGame();
  virtual void ReturnToLobby();

 protected:
  void OnAllClientsTravelReady() override;

  virtual void OpenCurrentScene() = 0;
  void RaceCountDown();
  void RaceStart();
  void ClearCountDown();
  void SaveResult(FNetworkConnectionId ConnectionId, float FinishTime);
  bool AreAllPlayersFinished() const;
  void TravelToClear();
  void RespawnNextItem();

  int m_CountDown = 3;
  FTimerHandle CountHandle;

  std::unordered_map<FNetworkConnectionId, MGhostRecorderComponent*> GhostRecorders;

  float RaceTime = 0.0f;
  bool RaceRunning = false;
  bool bPaused = false;
  bool bResultTravelRequested = false;
  float ResultTravelDelay = -1.0f;
  std::string MapId;
  int MapVersion = 1;
  std::string LevelFileName;
  FVector2D PlayerStartLocation;

  std::vector<FTimerHandle> m_itemRespawnHandles;
  std::vector<FVector2D> m_pendingRespawnLocations;
};
