#pragma once
#include "GameModeBase.h"
#include <vector>

class ALobbyPlayerState;

class AClearScene : public AGameModeBase
{
public:
	DEFINE_ACTOR_CLASS(AClearScene)

	AClearScene();
	void OnUpdate(float DeltaTime) override;

protected:
	void BeginPlay() override;

private:
	void SpawnResultStatesFromGameInstance();
};

