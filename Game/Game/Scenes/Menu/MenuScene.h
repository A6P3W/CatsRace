#pragma once

#include <optional>
#include <string>
#include <vector>

#include "GameModeBase.h"
#include "OnlinePlayManager.h"

class AWidgetBase;
class WCreateLobbyWidget;
class WMainMenuWidget;
class WJoinLobbyDialog;
class WSearchLobbyWidget;

class AMenuScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(AMenuScene)

  AMenuScene();
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  enum class EMenuState { MainMenu, OnlinePlay };

  void ShowMenuState(EMenuState NewState);
  void CloseActiveWidget();
  void UpdateActiveWidget();
  void UpdateActiveStatus();
  std::string BuildStatusText() const;

  bool StartHost();
  bool ConnectToHost();
  void CreateOnlineLobby();
  void TryCreateOnlineLobby();
  void SetCreateLobbyPending(bool bPending);
  void SearchOnlineLobbies();
  static constexpr float LobbySearchIntervalSeconds = 5.0f;
  void JoinSelectedOnlineLobby();
  void JoinOnlineLobby(const FLobbyInfo& LobbyInfo);
  void JoinOnlineLobbyAfterLatestCheck(const FLobbyInfo& LobbyInfo);
  void ShowJoinConfirmation(const FLobbyInfo& LobbyInfo);
  void LeaveOnlineLobby();
  void LoadSettings();
  void SaveSettings();

  char PlayerName[64] = "";
  char ServerAddress[64] = "127.0.0.1";
  std::string LobbyName = "Player Lobby";
  int Port = 7777;
  int OnlineLobbyMaxMembers = 8;
  int OnlineSearchMaxResults = 10;
  int SelectedOnlineLobbyIndex = -1;
  std::vector<FLobbyInfo> OnlineSearchResults;
  std::string StatusMessage = "Create or join a multiplayer session.";
  float LobbySearchRemaining = 0.0f;
  bool bLobbySearchInFlight = false;
  bool bCreateLobbyPending = false;
  std::optional<FHostLobbyRequest> PendingCreateLobbyRequest;
  std::string OnlineStatusMessage = "Online services are ready.";

  EMenuState CurrentState = EMenuState::MainMenu;
  AWidgetBase* ActiveWidget = nullptr;
  WMainMenuWidget* MainMenuWidget = nullptr;
  WCreateLobbyWidget* CreateLobbyWidget = nullptr;
  WSearchLobbyWidget* SearchLobbyWidget = nullptr;
  WJoinLobbyDialog* JoinLobbyDialog = nullptr;
  std::string PendingJoinLobbyId;
};
