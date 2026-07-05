#pragma once

#include <string>

#include "GameModeBase.h"

class UITextComponent;

class ALoadingScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(ALoadingScene)

  ALoadingScene();
  void BeginPlay() override;

 private:
  void SetStatusMessage(const std::string& Message);

  UITextComponent* StatusText = nullptr;
  char PlayerName[64] = "Player";
};