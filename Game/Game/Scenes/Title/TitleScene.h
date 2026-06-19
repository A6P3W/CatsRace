#pragma once
#include "GameModeBase.h"

class ATitleScene : public AGameModeBase
{
public:
	ATitleScene();
	void OnUpdate(float DeltaTime) override;
};

