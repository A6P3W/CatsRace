#pragma once
#include "GameModeBase.h"
#include <TimerHandle.h>
#include <TimerManager.h>

class WCountDown;
class WMainHUD;
class WPauseMenu;
class MGhostRecorderComponent;
class AGhostPlayer;

class AGameScene01 : public AGameModeBase
{
public:
	AGameScene01();
	void OnUpdate(float DeltaTime) override;
	void BeginPlay() override;
	float GetRaceTime() const { return RaceTime; }

	void RaceFinish();
	void TogglePause();
	void RestartGame();
	void ReturnToTitle();
private:
	void RaceCountDown();
	void RaceStart();
	void ClearCountDown();
	void LoadTopGhost();

	int m_CountDown = 3;
	FTimerHandle CountHandle;

	WCountDown* m_CountDownWidget = nullptr;
	WMainHUD* m_MainHUD = nullptr;
	WPauseMenu* m_PauseMenu = nullptr;
	MGhostRecorderComponent* m_GhostRecorder = nullptr;
	AGhostPlayer* m_GhostPlayer = nullptr;

	float RaceTime = 0.0f;
	bool RaceRunning = false;
	bool bPaused = false;
};
