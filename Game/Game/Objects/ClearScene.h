#pragma once
#include "GameModeBase.h"

class AClearScene : public AGameModeBase
{
public:
	AClearScene();
	void OnUpdate(float DeltaTime) override;
};

