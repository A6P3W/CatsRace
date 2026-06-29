#pragma once

#include <string>

#include "GameModeBase.h"

class AMenuScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(AMenuScene)

  AMenuScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;
  void Draw() override;

 private:
  void StartHost();
  void ConnectToHost();
  void LoadSettings();
  void SaveSettings();

  char PlayerName[64] = "Player";
  char ServerAddress[64] = "127.0.0.1";
  int Port = 7777;
  std::string StatusMessage = "Create or join a local session.";
};