#pragma once
#include "GameModeBase.h"

class WClearHUD;

class AClearScene : public AGameModeBase
{
public:
	AClearScene();
	void OnUpdate(float DeltaTime) override;

protected:
	void BeginPlay() override;

private:
	WClearHUD* m_ClearHUD = nullptr;
};

