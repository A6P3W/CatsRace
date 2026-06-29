#pragma once
#include <vector>

#include "GameModeBase.h"

class ALobbyPlayerState;

class AClearScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(AClearScene)

  AClearScene();
  void OnUpdate(float DeltaTime) override;

 protected:
  void BeginPlay() override;

 private:
  void SpawnResultStatesFromGameInstance();
};
