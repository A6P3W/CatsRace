#pragma once

#include <string>

#include "GameModeBase.h"

class UITextComponent;

class ALoadingScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(ALoadingScene)

  ALoadingScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void StartLevelDownload();
  void CompleteLevelLoading();
  void FailAndQuit(const std::string& Message);
  void SetStatusMessage(const std::string& Message);

  UITextComponent* StatusText = nullptr;
  char PlayerName[64] = "";
  float QuitCountdown = -1.0f;
  bool bFailed = false;
};
