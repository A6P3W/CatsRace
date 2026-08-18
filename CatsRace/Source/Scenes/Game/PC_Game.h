#pragma once
#include <TimerHandle.h>

#include <vector>

#include "NetworkManager.h"
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
  ~PC_Game() override;

  void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) override;
  void SetupInputMappings() override;

  void TogglePause();
  void RestartGame();
  void ReturnToLobby();
  void LeaveSession();
  void ReceiveRaceStartTime(double StartTime);

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void HandleNetworkPacket(FNetworkConnectionId ConnectionId, FNetBuffer& Buffer);
  void UpdateCountdown(double RemainingTime);
  void StartLocalRace();
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
  double RaceStartServerTime = 0.0;
  bool bHasRaceStartTime = false;
  int LastDisplayedCount = 0;
  FTimerHandle CountHandle;
  NetworkManager::CallbackHandle NetworkPacketCallbackHandle = 0;
};
