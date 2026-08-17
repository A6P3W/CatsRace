#pragma once
#include "GameSceneBase.h"

class AGameScene3 : public AGameSceneBase {
 public:
  DEFINE_ACTOR_CLASS(AGameScene3)
  AGameScene3();

 protected:
  void BeginPlay() override;
  void OpenCurrentScene() override;
};
