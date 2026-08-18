#pragma once
#include <TimerHandle.h>

#include <vector>

#include "PlayerController.h"

class EditorMode;
class MEnhancedInputComponent;
class WMainHUD;
class WCountDown;
class WControlGuide;
class WPauseMenu;
class APlayerDirectionIndicator;
class AGhostPlayer;

class PC_Game : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Game)

  PC_Game();

  void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) override;
  void SetupInputMappings() override;

  void TogglePause();
  void RestartGame();
  void ReturnToLobby();
  void LeaveSession();

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void RaceCountDown();
  void ClearCountDown();
  void SpawnRaceGhosts();

  WMainHUD* MainHUD = nullptr;
  WCountDown* CountDownWidget = nullptr;
  WControlGuide* ControlGuideWidget = nullptr;
  WPauseMenu* PauseMenu = nullptr;
  APlayerDirectionIndicator* PlayerDirectionIndicator = nullptr;
  std::vector<AGhostPlayer*> GhostPlayers;

  float RaceTime = 0.0f;
  bool RaceRunning = false;
  bool bPaused = false;
  int m_CountDown = 3;
  FTimerHandle CountHandle;
};
