#include "Scenes/Menu/MenuScene.h"

#include <Application.h>

#include <algorithm>
#include <cstring>
#include <utility>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"
#include "OnlinePlayManager.h"
#include "PlayerController.h"
#include "SceneManager.h"
#include "Scenes/Menu/UI/MenuWidgets.h"
#include "UIManager.h"
#include "World.h"

namespace {
constexpr const char* GameLobbyBucketId = "AGS2026Summer";
constexpr const char* LobbyStateAttributeKey = "LOBBY_STATE";
constexpr const char* LobbyStateWaiting = "WAITING";
constexpr const char* LobbyStateRacing = "RACING";

bool IsLobbyRacing(const FLobbyInfo& LobbyInfo) {
  return LobbyInfo.GetStringAttribute(LobbyStateAttributeKey, LobbyStateWaiting) ==
         LobbyStateRacing;
}
}  // namespace

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
  UpdateActiveStatus();
  if (CurrentState != EMenuState::OnlinePlay) {
    return;
  }

  TryCreateOnlineLobby();
  LobbySearchRemaining -= DeltaTime;
  if (LobbySearchRemaining <= 0.0f && !bLobbySearchInFlight && !bCreateLobbyPending &&
      !JoinLobbyDialog && OnlinePlayManager::GetInstance().CanSearchLobbies()) {
    SearchOnlineLobbies();
  }
}

void AMenuScene::ShowMenuState(EMenuState NewState) {
  CloseActiveWidget();
  CurrentState = NewState;

  switch (CurrentState) {
    case EMenuState::OnlinePlay:
      CreateLobbyWidget = SpawnActor<WCreateLobbyWidget>();
      CreateLobbyWidget->SetInitialLobbyName(LobbyName);
      CreateLobbyWidget->OnLobbyNameChanged = [this](const std::string& NewLobbyName) {
        LobbyName = NewLobbyName.empty() ? "Player Lobby" : NewLobbyName;
      };
      CreateLobbyWidget->OnCreate = [this]() { CreateOnlineLobby(); };
      CreateLobbyWidget->OnBack = [this]() { ShowMenuState(EMenuState::MainMenu); };
      CreateLobbyWidget->OnFocusSearchResults = [this]() {
        if (SearchLobbyWidget && SearchLobbyWidget->GetFirstJoinableButton()) {
          CreateLobbyWidget->ClearFocusedButton();
          UIManager::GetInstance()->SetFocusedWidget(SearchLobbyWidget);
          SearchLobbyWidget->FocusFirstJoinableButton();
        }
      };

      SearchLobbyWidget = SpawnActor<WSearchLobbyWidget>();
      SearchLobbyWidget->OnBack = [this]() { ShowMenuState(EMenuState::MainMenu); };
      SearchLobbyWidget->OnFocusCreate = [this]() {
        if (CreateLobbyWidget) {
          SearchLobbyWidget->ClearFocusedButton();
          UIManager::GetInstance()->SetFocusedWidget(CreateLobbyWidget);
          CreateLobbyWidget->FocusCreateButton();
        }
      };
      SearchLobbyWidget->OnLobbySelected = [this](int LobbyIndex) {
        SelectedOnlineLobbyIndex = LobbyIndex;
        ShowJoinConfirmation(OnlineSearchResults[LobbyIndex]);
      };
      ActiveWidget = CreateLobbyWidget;
      break;
    case EMenuState::MainMenu:
    default:
      MainMenuWidget = SpawnActor<WMainMenuWidget>();
      MainMenuWidget->SetInitialUserName(PlayerName);
      MainMenuWidget->OnUserNameChanged = [this](const std::string& NewName) {
        strncpy_s(PlayerName, sizeof(PlayerName), NewName.c_str(), _TRUNCATE);
        SaveSettings();
        LobbyName = std::string(PlayerName) + "'s Lobby";
      };
      MainMenuWidget->OnCreateLobby = [this]() { ShowMenuState(EMenuState::OnlinePlay); };
      MainMenuWidget->OnQuitGame = []() { Application::QuitGame(); };
      ActiveWidget = MainMenuWidget;
      break;
  }

  if (ActiveWidget) {
    UIManager::GetInstance()->AddWidget(ActiveWidget);
    if (SearchLobbyWidget) {
      UIManager::GetInstance()->AddWidget(SearchLobbyWidget);
    }
    UIManager::GetInstance()->SetFocusedWidget(ActiveWidget);
  }

  UpdateActiveWidget();
  if (CurrentState == EMenuState::OnlinePlay) {
    LobbySearchRemaining = 0.0f;
    SearchOnlineLobbies();
  }
}

void AMenuScene::CloseActiveWidget() {
  bCreateLobbyPending = false;
  PendingCreateLobbyRequest.reset();
  if (JoinLobbyDialog) {
    UIManager::GetInstance()->RemoveWidget(JoinLobbyDialog);
    JoinLobbyDialog = nullptr;
    PendingJoinLobbyId.clear();
  }
  if (ActiveWidget) {
    UIManager::GetInstance()->RemoveWidget(ActiveWidget);
  }
  if (SearchLobbyWidget && SearchLobbyWidget != ActiveWidget) {
    UIManager::GetInstance()->RemoveWidget(SearchLobbyWidget);
  }
  ActiveWidget = nullptr;
  MainMenuWidget = nullptr;
  CreateLobbyWidget = nullptr;
  SearchLobbyWidget = nullptr;
}

void AMenuScene::UpdateActiveWidget() {
  UpdateActiveStatus();

  if (JoinLobbyDialog) {
    const bool bPendingLobbyCanStillJoin = std::any_of(
        OnlineSearchResults.begin(),
        OnlineSearchResults.end(),
        [this](const FLobbyInfo& LobbyInfo) {
          return LobbyInfo.LobbyId == PendingJoinLobbyId && !IsLobbyRacing(LobbyInfo);
        }
    );
    if (!bPendingLobbyCanStillJoin) {
      UIManager::GetInstance()->RemoveWidget(JoinLobbyDialog);
      JoinLobbyDialog = nullptr;
      PendingJoinLobbyId.clear();
      if (SearchLobbyWidget) {
        SearchLobbyWidget->ClearFocusedButton();
      }
      if (CreateLobbyWidget) {
        UIManager::GetInstance()->SetFocusedWidget(CreateLobbyWidget);
        CreateLobbyWidget->FocusCreateButton();
      }
    }
  }

  if (SearchLobbyWidget) {
    SearchLobbyWidget->SetLobbyResults(OnlineSearchResults, SelectedOnlineLobbyIndex);
    if (!JoinLobbyDialog && UIManager::GetInstance()->GetFocusedWidget() == SearchLobbyWidget) {
      SearchLobbyWidget->FocusFirstJoinableButton();
    }
    if (CreateLobbyWidget) {
      CreateLobbyWidget->SetSearchNavigation(SearchLobbyWidget->GetFirstJoinableButton());
    }
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
  if (bCreateLobbyPending) {
    return;
  }

  SaveSettings();
  const std::string safeLobbyName = LobbyName.empty() ? "Player Lobby" : LobbyName;

  FHostLobbyRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxMembers = (std::max)(1, OnlineLobbyMaxMembers);
  request.PublicAdvertised = true;
  request.Port = static_cast<uint16_t>(Port);
  request.Attributes.push_back(
      {"LOBBYNAME", FLobbyAttributeValue::FromString(safeLobbyName), true}
  );
  request.Attributes.push_back({"HOSTNAME", FLobbyAttributeValue::FromString(PlayerName), true});
  request.Attributes.push_back(
      {LobbyStateAttributeKey, FLobbyAttributeValue::FromString(LobbyStateWaiting), true}
  );

  PendingCreateLobbyRequest = std::move(request);
  SetCreateLobbyPending(true);
  OnlineStatusMessage = bLobbySearchInFlight ? "Waiting for lobby search to finish before creating."
                                             : "Create online lobby requested.";
  UpdateActiveStatus();
  TryCreateOnlineLobby();
}
void AMenuScene::TryCreateOnlineLobby() {
  if (!bCreateLobbyPending || !PendingCreateLobbyRequest || bLobbySearchInFlight) {
    return;
  }

  const FHostLobbyRequest request = *PendingCreateLobbyRequest;
  OnlineStatusMessage = "Create online lobby requested.";
  UpdateActiveStatus();
  OnlinePlayManager::GetInstance().HostLobby(request, [this](const FOnlinePlayResult& Result) {
    SetCreateLobbyPending(false);
    OnlineStatusMessage = Result.Message;
    if (!Result.Success) {
      UpdateActiveStatus();
      return;
    }

    OnlineSearchResults.clear();
    SelectedOnlineLobbyIndex = -1;
    if (SceneManager::GetInstance().OpenLevelById(GameSceneIds::Lobby, ENetMode::ListenServer)) {
      OnlineStatusMessage += " LobbyId=" + Result.LobbyInfo.LobbyId;
      UpdateActiveStatus();
      return;
    }

    OnlineStatusMessage = "Lobby host is ready, but the lobby scene failed to open.";
    UpdateActiveStatus();
    OnlinePlayManager::GetInstance().LeaveLobby([this](const FOnlinePlayResult& LeaveResult) {
      if (!LeaveResult.Success) {
        OnlineStatusMessage += " " + LeaveResult.Message;
        UpdateActiveStatus();
      }
    });
  });
}
void AMenuScene::SetCreateLobbyPending(bool bPending) {
  bCreateLobbyPending = bPending;
  if (!bPending) {
    PendingCreateLobbyRequest.reset();
    if (CurrentState == EMenuState::OnlinePlay && CreateLobbyWidget) {
      UIManager::GetInstance()->SetFocusedWidget(CreateLobbyWidget);
      CreateLobbyWidget->FocusCreateButton();
    }
    return;
  }

  if (CreateLobbyWidget) {
    CreateLobbyWidget->ClearFocusedButton();
  }
  if (SearchLobbyWidget) {
    SearchLobbyWidget->ClearFocusedButton();
  }
  UIManager::GetInstance()->SetFocusedWidget(nullptr);
}

void AMenuScene::SearchOnlineLobbies() {
  if (CurrentState != EMenuState::OnlinePlay || bLobbySearchInFlight) {
    return;
  }

  FLobbySearchRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxResults = (std::max)(1, OnlineSearchMaxResults);

  bLobbySearchInFlight = true;
  LobbySearchRemaining = LobbySearchIntervalSeconds;
  OnlineStatusMessage = "Search lobbies requested.";
  UpdateActiveStatus();
  OnlinePlayManager::GetInstance().SearchLobbies(
      request, [this](const FOnlinePlayResult& Result, const std::vector<FLobbyInfo>& Results) {
        bLobbySearchInFlight = false;
        OnlineStatusMessage = Result.Message;
        if (Result.Success) {
          OnlineSearchResults = Results;
          SelectedOnlineLobbyIndex = OnlineSearchResults.empty() ? -1 : 0;
          OnlineStatusMessage += " Found: " + std::to_string(OnlineSearchResults.size());
        }
        UpdateActiveWidget();
        TryCreateOnlineLobby();
      }
  );
}
void AMenuScene::JoinSelectedOnlineLobby() {
  if (SelectedOnlineLobbyIndex < 0 ||
      SelectedOnlineLobbyIndex >= static_cast<int>(OnlineSearchResults.size())) {
    OnlineStatusMessage = "Select a lobby search result first.";
    UpdateActiveStatus();
    return;
  }

  const FLobbyInfo lobbyInfo = OnlineSearchResults[SelectedOnlineLobbyIndex];
  if (IsLobbyRacing(lobbyInfo)) {
    OnlineStatusMessage = "レース中のため参加できません。";
    UpdateActiveStatus();
    return;
  }

  JoinOnlineLobby(lobbyInfo);
}
void AMenuScene::JoinOnlineLobbyAfterLatestCheck(const FLobbyInfo& lobbyInfo) {
  if (IsLobbyRacing(lobbyInfo)) {
    OnlineStatusMessage = "レース中のため参加できません。";
    UpdateActiveStatus();
    SearchOnlineLobbies();
    return;
  }
  JoinOnlineLobby(lobbyInfo);
}
void AMenuScene::JoinOnlineLobby(const FLobbyInfo& lobbyInfo) {
  SaveSettings();
  OnlineStatusMessage = "Join lobby requested: " + lobbyInfo.LobbyId;
  UpdateActiveStatus();
  OnlinePlayManager::GetInstance().JoinLobby(
      lobbyInfo, static_cast<uint16_t>(Port), [this, lobbyInfo](const FOnlinePlayResult& Result) {
        OnlineStatusMessage = Result.Message;
        if (!Result.Success) {
          UpdateActiveStatus();
          LobbySearchRemaining = 0.0f;
          return;
        }

        if (!lobbyInfo.HostIPAddress.empty()) {
          strncpy_s(
              ServerAddress, sizeof(ServerAddress), lobbyInfo.HostIPAddress.c_str(), _TRUNCATE
          );
          SaveSettings();
        }
        StatusMessage = Result.Message;
        OnlineStatusMessage += " LobbyId=" + Result.LobbyInfo.LobbyId;
        UpdateActiveStatus();
      }
  );
}
void AMenuScene::LeaveOnlineLobby() {
  const std::string lobbyId = OnlinePlayManager::GetInstance().GetCurrentLobbyId();
  OnlineStatusMessage = "Leave lobby requested: " + lobbyId;
  UpdateActiveStatus();
  OnlinePlayManager::GetInstance().LeaveLobby([this, lobbyId](const FOnlinePlayResult& Result) {
    OnlineStatusMessage = Result.Message;
    if (Result.Success) {
      StatusMessage = "Left lobby.";
      OnlineStatusMessage += " LobbyId=" + lobbyId;
    }
    UpdateActiveStatus();
  });
}
void AMenuScene::LoadSettings() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    if (gi->player_name.empty()) {
      gi->player_name = gi->user_id.empty() ? PlayerNameDefaults::Generate() : gi->user_id;
    }
    strncpy_s(PlayerName, sizeof(PlayerName), gi->player_name.c_str(), _TRUNCATE);
    LobbyName = gi->player_name + "'s Lobby";

    if (!gi->last_server_ip.empty()) {
      strncpy_s(ServerAddress, sizeof(ServerAddress), gi->last_server_ip.c_str(), _TRUNCATE);
    }
  }
}

void AMenuScene::SaveSettings() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    if (PlayerName[0] != '\0') {
      gi->player_name = PlayerName;
    } else if (gi->player_name.empty()) {
      gi->player_name = PlayerNameDefaults::Generate();
    }
    if (PlayerName[0] == '\0') {
      strncpy_s(PlayerName, sizeof(PlayerName), gi->player_name.c_str(), _TRUNCATE);
    }
    gi->last_server_ip = ServerAddress;
  }
}

void AMenuScene::ShowJoinConfirmation(const FLobbyInfo& LobbyInfo) {
  if (JoinLobbyDialog) return;
  JoinLobbyDialog = SpawnActor<WJoinLobbyDialog>();
  if (CreateLobbyWidget) {
    CreateLobbyWidget->ClearFocusedButton();
  }
  if (SearchLobbyWidget) {
    SearchLobbyWidget->ClearFocusedButton();
  }
  PendingJoinLobbyId = LobbyInfo.LobbyId;
  JoinLobbyDialog->SetLobbyName(LobbyInfo.GetStringAttribute("LOBBYNAME", "Unknown Lobby"));
  UIManager* uiManager = UIManager::GetInstance();
  JoinLobbyDialog->OnJoin = [this, LobbyInfo]() {
    UIManager* uiManager = UIManager::GetInstance();
    uiManager->RemoveWidget(JoinLobbyDialog);
    JoinLobbyDialog = nullptr;
    PendingJoinLobbyId.clear();
    if (SearchLobbyWidget) {
      SearchLobbyWidget->ClearFocusedButton();
    }
    if (CreateLobbyWidget) {
      uiManager->SetFocusedWidget(CreateLobbyWidget);
      CreateLobbyWidget->FocusCreateButton();
    }
    JoinOnlineLobbyAfterLatestCheck(LobbyInfo);
  };
  JoinLobbyDialog->OnBack = [this, LobbyInfo]() {
    UIManager* uiManager = UIManager::GetInstance();
    uiManager->RemoveWidget(JoinLobbyDialog);
    JoinLobbyDialog = nullptr;
    PendingJoinLobbyId.clear();
    if (SearchLobbyWidget) {
      SearchLobbyWidget->ClearFocusedButton();
    }
    if (CreateLobbyWidget) {
      uiManager->SetFocusedWidget(CreateLobbyWidget);
      CreateLobbyWidget->FocusCreateButton();
    }
  };
  uiManager->AddWidget(JoinLobbyDialog);
  uiManager->SetFocusedWidget(JoinLobbyDialog);
}
