#pragma once

#include <string>
#include <vector>

#include "EOSTypes.h"
#include "GameModeBase.h"

class AMenuScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(AMenuScene)

  AMenuScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;
  void Draw() override;

 private:
  bool StartHost();
  bool ConnectToHost();
  void LoginWithDeviceId();
  void CreateOnlineLobby();
  void SearchOnlineLobbies();
  void JoinSelectedOnlineLobby();
  void LeaveOnlineLobby();
  void LoadSettings();
  void SaveSettings();

  char PlayerName[64] = "Player";
  char ServerAddress[64] = "127.0.0.1";
  int Port = 7777;
  int OnlineLobbyMaxMembers = 4;
  int OnlineSearchMaxResults = 10;
  int SelectedOnlineLobbyIndex = -1;
  std::vector<FLobbyInfo> OnlineSearchResults;
  std::string StatusMessage = "Create or join a multiplayer session.";
  std::string OnlineStatusMessage = "Login to use EOS LAN lobbies.";
};
