#pragma once
#include <TimerHandle.h>

#include "PlayerController.h"

class EditorMode;
class MEnhancedInputComponent;
class WMainHUD;
class WCountDown;
class WPauseMenu;

class PC_Game : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Game)

  PC_Game();

  void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) override;
  void SetupInputMappings() override;

  void TogglePause();
  void RestartGame();
  void ReturnToTitle();

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void RaceCountDown();
  void ClearCountDown();

  WMainHUD* m_MainHUD = nullptr;
  WCountDown* m_CountDownWidget = nullptr;
  WPauseMenu* m_PauseMenu = nullptr;

  float RaceTime = 0.0f;
  bool RaceRunning = false;
  bool bPaused = false;
  int m_CountDown = 3;
  FTimerHandle CountHandle;
};
