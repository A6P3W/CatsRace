#pragma once
#include "GameModeBase.h"
#include <TimerHandle.h>
#include <TimerManager.h>

class WCountDown;
class WMainHUD;

class AGameScene01 : public AGameModeBase
{
public:
	AGameScene01();
	void OnUpdate(float DeltaTime) override;
	void BeginPlay() override;
	float GetRaceTime() const { return RaceTime; }

	void RaceFinish();
private:
	void RaceCountDown();
	void RaceStart();
	void ClearCountDown();

	int m_CountDown = 3;
	FTimerHandle CountHandle;

	WCountDown* m_CountDownWidget = nullptr;
	WMainHUD* m_MainHUD = nullptr;

	float RaceTime = 0.0f;
	bool RaceRunning = false;
};
