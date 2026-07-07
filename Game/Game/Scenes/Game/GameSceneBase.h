#pragma once
#include <TimerHandle.h>
#include <TimerManager.h>

#include <string>
#include <vector>

#include "GameModeBase.h"

class WCountDown;
class WMainHUD;
class WPauseMenu;
class MGhostRecorderComponent;
class AGhostPlayer;
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

  float GetRaceTime() const { return RaceTime; }
  const std::string& GetMapId() const { return MapId; }

  void RaceFinish();
  void NotifyPlayerFinished(APlayer* Player);
  virtual void RestartGame();
  virtual void ReturnToTitle();

 protected:
  virtual void OpenCurrentScene() = 0;
  void RaceCountDown();
  void RaceStart();
  void ClearCountDown();
  void LoadTopGhost();
  void SaveResult(FNetworkConnectionId ConnectionId, float FinishTime);
  bool AreAllPlayersFinished() const;
  void TravelToClear();

  int m_CountDown = 3;
  FTimerHandle CountHandle;

  MGhostRecorderComponent* m_GhostRecorder = nullptr;
  AGhostPlayer* m_GhostPlayer = nullptr;

  float RaceTime = 0.0f;
  bool RaceRunning = false;
  bool bPaused = false;
  bool bResultTravelRequested = false;
  float ResultTravelDelay = -1.0f;
  std::string MapId;
  std::string LevelFileName;
  FVector2D PlayerStartLocation;
};
