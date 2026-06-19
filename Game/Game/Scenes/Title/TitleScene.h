#pragma once
#include "GameModeBase.h"

class WTitleHUD;

class ATitleScene : public AGameModeBase
{
public:
	ATitleScene();
	void BeginPlay() override;

private:
	WTitleHUD* m_TitleHUD = nullptr;
};

