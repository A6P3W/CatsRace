#pragma once
#include "GameSceneBase.h"

class AGameScene2 : public AGameSceneBase {
 public:
  DEFINE_ACTOR_CLASS(AGameScene2)
  AGameScene2();

 protected:
  void BeginPlay() override;
  void OpenCurrentScene() override;
};
