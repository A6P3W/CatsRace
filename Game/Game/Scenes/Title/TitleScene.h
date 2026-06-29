#pragma once
#include "GameModeBase.h"

class WTitleHUD;

class ATitleScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(ATitleScene)

  ATitleScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  WTitleHUD* m_TitleHUD = nullptr;
};
