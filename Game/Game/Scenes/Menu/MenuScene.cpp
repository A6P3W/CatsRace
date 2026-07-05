#include "Scenes/Menu/MenuScene.h"

#include <Application.h>

#include <algorithm>
#include <cstring>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"
#include "NetworkUtils.h"
#include "OnlineSessionManager.h"
#include "PlayerController.h"
#include "SceneManager.h"
#include "Scenes/Menu/UI/MenuWidgets.h"
#include "UIManager.h"
#include "World.h"

namespace {
constexpr const char* GameLobbyBucketId = "AGS2026Summer";
}

REGISTER_GAME_MODE(AMenuScene)

AMenuScene::AMenuScene() { SetUpdateableAnytime(true); }

void AMenuScene::BeginPlay() {
  AGameModeBase::BeginPlay();
  LoadSettings();

  if (auto* controller = GetWorld()->GetOrCreateLocalPlayerController()) {
    controller->SetInputMode(EInputMode::UIOnly);
  }

  ShowMenuState(EMenuState::MainMenu);
}

void AMenuScene::OnUpdate(float DeltaTime) {
  (void)DeltaTime;
  UpdateActiveStatus();
}

void AMenuScene::ShowMenuState(EMenuState NewState) {
  CloseActiveWidget();
  CurrentState = NewState;

  switch (CurrentState) {
    case EMenuState::CreateLobby:
      CreateLobbyWidget = SpawnActor<WCreateLobbyWidget>();
      CreateLobbyWidget->SetInitialLobbyName(LobbyName);
      CreateLobbyWidget->OnLobbyNameChanged = [this](const std::string& NewLobbyName) {
        LobbyName = NewLobbyName.empty() ? "Player Lobby" : NewLobbyName;
      };
      CreateLobbyWidget->OnCreate = [this]() { CreateOnlineLobby(); };
      CreateLobbyWidget->OnBack = [this]() { ShowMenuState(EMenuState::MainMenu); };
      ActiveWidget = CreateLobbyWidget;
      break;
    case EMenuState::SearchLobby:
      SearchLobbyWidget = SpawnActor<WSearchLobbyWidget>();
      SearchLobbyWidget->OnRefresh = [this]() { SearchOnlineLobbies(); };
      SearchLobbyWidget->OnBack = [this]() { ShowMenuState(EMenuState::MainMenu); };
      SearchLobbyWidget->OnLobbySelected = [this](int LobbyIndex) {
        SelectedOnlineLobbyIndex = LobbyIndex;
        JoinSelectedOnlineLobby();
      };
      ActiveWidget = SearchLobbyWidget;
      break;
    case EMenuState::MainMenu:
    default:
      MainMenuWidget = SpawnActor<WMainMenuWidget>();
      MainMenuWidget->OnCreateLobby = [this]() { ShowMenuState(EMenuState::CreateLobby); };
      MainMenuWidget->OnSearchLobby = [this]() { ShowMenuState(EMenuState::SearchLobby); };
      MainMenuWidget->OnQuitGame = []() { Application::QuitGame(); };
      ActiveWidget = MainMenuWidget;
      break;
  }

  if (ActiveWidget) {
    UIManager::GetInstance()->AddWidget(ActiveWidget);
    UIManager::GetInstance()->SetFocusedWidget(ActiveWidget);
  }

  UpdateActiveWidget();
  if (CurrentState == EMenuState::SearchLobby) {
    SearchOnlineLobbies();
  }
}

void AMenuScene::CloseActiveWidget() {
  if (ActiveWidget) {
    UIManager::GetInstance()->RemoveWidget(ActiveWidget);
  }
  ActiveWidget = nullptr;
  MainMenuWidget = nullptr;
  CreateLobbyWidget = nullptr;
  SearchLobbyWidget = nullptr;
}

void AMenuScene::UpdateActiveWidget() {
  UpdateActiveStatus();

  if (SearchLobbyWidget) {
    SearchLobbyWidget->SetLobbyResults(OnlineSearchResults, SelectedOnlineLobbyIndex);
  }
}

void AMenuScene::UpdateActiveStatus() {
  const std::string statusText = BuildStatusText();
  if (MainMenuWidget) {
    MainMenuWidget->SetStatusText(statusText);
  }
  if (CreateLobbyWidget) {
    CreateLobbyWidget->SetStatusText(statusText);
  }
  if (SearchLobbyWidget) {
    SearchLobbyWidget->SetStatusText(statusText);
  }
}

std::string AMenuScene::BuildStatusText() const {
  if (OnlineStatusMessage.empty()) {
    return StatusMessage;
  }
  if (StatusMessage.empty()) {
    return OnlineStatusMessage;
  }
  return OnlineStatusMessage + " / " + StatusMessage;
}

bool AMenuScene::StartHost() {
  SaveSettings();
  NetworkManager& network = NetworkManager::GetInstance();
  if (!network.StartServer(static_cast<uint16_t>(Port))) {
    StatusMessage = "Failed to start listen server.";
    UpdateActiveStatus();
    return false;
  }

  GetWorld()->SetNetMode(ENetMode::ListenServer);
  if (!SceneManager::GetInstance().OpenLevelById(GameSceneIds::Lobby, ENetMode::ListenServer)) {
    StatusMessage = "Failed to open lobby scene.";
    UpdateActiveStatus();
    return false;
  }
  StatusMessage = "Listen server started.";
  UpdateActiveStatus();
  return true;
}

bool AMenuScene::ConnectToHost() {
  SaveSettings();
  NetworkManager& network = NetworkManager::GetInstance();
  if (!network.ConnectToServer(ServerAddress, static_cast<uint16_t>(Port))) {
    StatusMessage = "Failed to start client connection.";
    UpdateActiveStatus();
    return false;
  }

  GetWorld()->SetNetMode(ENetMode::Client);
  StatusMessage = "Connecting. Waiting for server travel...";
  UpdateActiveStatus();
  return true;
}

void AMenuScene::CreateOnlineLobby() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before creating a lobby.";
    UpdateActiveStatus();
    return;
  }

  if (OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Already in a lobby. Leave it before creating another one.";
    UpdateActiveStatus();
    return;
  }

  SaveSettings();

  const std::string localIPAddress = NetworkUtils::GetLocalIPAddress();
  if (localIPAddress.empty()) {
    OnlineStatusMessage = "Local IP address was not found.";
    UpdateActiveStatus();
    return;
  }

  const std::string safeLobbyName = LobbyName.empty() ? "Player Lobby" : LobbyName;

  FCreateLobbyRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxMembers = (std::max)(1, OnlineLobbyMaxMembers);
  request.bPublicAdvertised = true;
  request.HostIPAddress = localIPAddress;
  request.Attributes.push_back({"LOBBYNAME", FLobbyAttributeValue::FromString(safeLobbyName), true});
  request.Attributes.push_back({"HOSTNAME", FLobbyAttributeValue::FromString(PlayerName), true});

  OnlineStatusMessage = "Create online lobby requested. HostIP=" + localIPAddress;
  UpdateActiveStatus();
  if (!OnlineSessionManager::Get().CreateLobby(
          request, [this](bool bSuccess, const FLobbyInfo& lobbyInfo) {
            if (!bSuccess) {
              OnlineStatusMessage = "Create online lobby failed. See Logs for details.";
              UpdateActiveStatus();
              return;
            }

            OnlineSearchResults.clear();
            SelectedOnlineLobbyIndex = -1;
            if (StartHost()) {
              OnlineStatusMessage =
                  "Online lobby created: " + lobbyInfo.LobbyId + " HostIP=" + lobbyInfo.HostIPAddress;
              UpdateActiveStatus();
              return;
            }

            OnlineStatusMessage = "Online lobby created, but listen server failed to start.";
            UpdateActiveStatus();
            OnlineSessionManager::Get().LeaveLobby([this](bool bLeaveSuccess) {
              if (!bLeaveSuccess) {
                OnlineStatusMessage += " Leave lobby also failed. See Logs for details.";
                UpdateActiveStatus();
              }
            });
          }
      )) {
    OnlineStatusMessage = "Create online lobby request was rejected.";
    UpdateActiveStatus();
  }
}

void AMenuScene::SearchOnlineLobbies() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before searching lobbies.";
    UpdateActiveStatus();
    return;
  }

  FLobbySearchRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxResults = (std::max)(1, OnlineSearchMaxResults);

  OnlineSearchResults.clear();
  SelectedOnlineLobbyIndex = -1;
  OnlineStatusMessage = "Search lobbies requested.";
  UpdateActiveWidget();
  if (!OnlineSessionManager::Get().SearchLobbies(
          request, [this](bool bSuccess, const std::vector<FLobbyInfo>& results) {
            if (bSuccess) {
              OnlineSearchResults = results;
              SelectedOnlineLobbyIndex = OnlineSearchResults.empty() ? -1 : 0;
              OnlineStatusMessage =
                  "Lobby search completed. Found: " + std::to_string(OnlineSearchResults.size());
            } else {
              OnlineStatusMessage = "Lobby search failed. See Logs for details.";
            }
            UpdateActiveWidget();
          }
      )) {
    OnlineStatusMessage = "Lobby search request was rejected.";
    UpdateActiveWidget();
  }
}

void AMenuScene::JoinSelectedOnlineLobby() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before joining a lobby.";
    UpdateActiveStatus();
    return;
  }

  if (OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Already in a lobby. Leave it before joining another one.";
    UpdateActiveStatus();
    return;
  }

  if (SelectedOnlineLobbyIndex < 0 ||
      SelectedOnlineLobbyIndex >= static_cast<int>(OnlineSearchResults.size())) {
    OnlineStatusMessage = "Select a lobby search result first.";
    UpdateActiveStatus();
    return;
  }

  const FLobbyInfo lobbyInfo = OnlineSearchResults[SelectedOnlineLobbyIndex];
  if (lobbyInfo.HostIPAddress.empty()) {
    OnlineStatusMessage = "Selected lobby does not have HostIP.";
    UpdateActiveStatus();
    return;
  }

  SaveSettings();

  OnlineStatusMessage = "Join lobby requested: " + lobbyInfo.LobbyId;
  UpdateActiveStatus();
  if (!OnlineSessionManager::Get().JoinLobby(
          lobbyInfo, static_cast<uint16_t>(Port), [this, lobbyInfo](bool bSuccess) {
            if (!bSuccess) {
              OnlineStatusMessage =
                  "Join lobby or ENet connection failed. Search again if the result is stale.";
              UpdateActiveStatus();
              return;
            }

            strncpy_s(
                ServerAddress, sizeof(ServerAddress), lobbyInfo.HostIPAddress.c_str(), _TRUNCATE
            );
            SaveSettings();
            if (GetWorld()) {
              GetWorld()->SetNetMode(ENetMode::Client);
            }
            StatusMessage = "Connecting to lobby host: " + lobbyInfo.HostIPAddress;
            OnlineStatusMessage = "Joined lobby and connecting: " + lobbyInfo.LobbyId;
            UpdateActiveStatus();
          }
      )) {
    OnlineStatusMessage = "Join lobby request was rejected.";
    UpdateActiveStatus();
  }
}

void AMenuScene::LeaveOnlineLobby() {
  if (!OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Not in a lobby.";
    UpdateActiveStatus();
    return;
  }

  const std::string lobbyId = OnlineSessionManager::Get().GetCurrentLobbyId();
  OnlineStatusMessage = "Leave lobby requested: " + lobbyId;
  UpdateActiveStatus();
  if (!OnlineSessionManager::Get().LeaveLobby([this, lobbyId](bool bSuccess) {
        if (bSuccess) {
          if (GetWorld()) {
            GetWorld()->SetNetMode(ENetMode::Standalone);
          }
          StatusMessage = "Left lobby.";
          OnlineStatusMessage = "Left lobby: " + lobbyId;
        } else {
          OnlineStatusMessage = "Leave lobby failed. See Logs for details.";
        }
        UpdateActiveStatus();
      })) {
    OnlineStatusMessage = "Leave lobby request was rejected.";
    UpdateActiveStatus();
  }
}

void AMenuScene::LoadSettings() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    const std::string name = gi->player_name.empty() ? gi->user_id : gi->player_name;
    if (!name.empty()) {
      strncpy_s(PlayerName, sizeof(PlayerName), name.c_str(), _TRUNCATE);
      LobbyName = name + "'s Lobby";
    }
    if (!gi->last_server_ip.empty()) {
      strncpy_s(ServerAddress, sizeof(ServerAddress), gi->last_server_ip.c_str(), _TRUNCATE);
    }
  }
}

void AMenuScene::SaveSettings() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    gi->player_name = PlayerName[0] == '\0' ? "Player" : PlayerName;
    gi->last_server_ip = ServerAddress;
  }
}
