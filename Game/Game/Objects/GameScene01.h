#pragma once
#include "GameModeBase.h"
#include <TimerHandle.h>
#include <TimerManager.h>

class MSpriteComponent;
class AGameScene01 :public AGameModeBase
{
public:
	AGameScene01();
	void OnUpdate(float DeltaTime) override;
	void BeginPlay() override;
private:
	void RaceCountDown();
	void RaceStart();
	int m_CountDown = 3;
	FTimerHandle CountHandle;

	MSpriteComponent* m_CountDownSprite = nullptr;

	void ClearCountDownSprite();


};

