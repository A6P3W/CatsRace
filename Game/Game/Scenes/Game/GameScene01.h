#pragma once
#include "GameSceneBase.h"

class AGameScene01 : public AGameSceneBase
{
public:
	DEFINE_ACTOR_CLASS(AGameScene01)
	AGameScene01();

protected:
	void OpenCurrentScene() override;
};
