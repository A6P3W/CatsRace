#include "Scenes/Menu/MenuScene.h"

#include <imgui.h>

#include <algorithm>
#include <cstring>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"
#include "NetworkUtils.h"
#include "OnlineSessionManager.h"
#include "SceneManager.h"
#include "Scenes/Common/ImGuiHelper.h"
#include "World.h"

namespace {
constexpr const char* GameLobbyBucketId = "AGS2026Summer";
}

REGISTER_GAME_MODE(AMenuScene)

AMenuScene::AMenuScene() { SetUpdateableAnytime(true); }

void AMenuScene::BeginPlay() {
  AGameModeBase::BeginPlay();
  LoadSettings();
}

void AMenuScene::OnUpdate(float DeltaTime) { (void)DeltaTime; }

void AMenuScene::Draw() {
  AGameModeBase::Draw();

  OnlineSessionManager& onlineSession = OnlineSessionManager::Get();

  ImGui::SetNextWindowSize(ImVec2(620.0f, 560.0f), ImGuiCond_FirstUseEver);
  ImGui::Begin("Multiplayer Menu");
  ImGuiHelper::DrawCenteredStatusText("Cats Race");
  ImGui::Separator();

  ImGui::InputText("Player Name", PlayerName, sizeof(PlayerName));
  ImGui::InputInt("Port", &Port);
  Port = std::clamp(Port, 1, 65535);
  OnlineLobbyMaxMembers = std::clamp(OnlineLobbyMaxMembers, 1, 8);
  OnlineSearchMaxResults = std::clamp(OnlineSearchMaxResults, 1, 100);

  ImGui::Separator();
  ImGui::TextUnformatted("EOS LAN Lobby");
  ImGui::Text("EOS initialized: %s", onlineSession.IsEOSInitialized() ? "true" : "false");
  ImGui::Text("Logged in: %s", onlineSession.IsLoggedIn() ? "true" : "false");
  if (onlineSession.IsLoggedIn()) {
    const std::string localUserId = onlineSession.GetLocalUserIdString();
    ImGui::TextWrapped("ProductUserId: %s", localUserId.c_str());
  }
  ImGui::Text("In lobby: %s", onlineSession.IsInLobby() ? "true" : "false");
  if (onlineSession.IsInLobby()) {
    const std::string lobbyId = onlineSession.GetCurrentLobbyId();
    ImGui::TextWrapped("Current LobbyId: %s", lobbyId.c_str());
  }
  ImGui::Text("Operation pending: %s", onlineSession.IsOperationPending() ? "true" : "false");

  if (!onlineSession.IsEOSInitialized()) {
    ImGui::TextDisabled("EOS is not initialized.");
  } else if (!onlineSession.IsLoggedIn()) {
    if (onlineSession.CanShowLogin() && ImGui::Button("Login Device ID", ImVec2(180.0f, 34.0f))) {
      LoginWithDeviceId();
    }
  } else if (onlineSession.IsInLobby()) {
    if (onlineSession.CanShowLeaveLobby() && ImGui::Button("Leave Lobby", ImVec2(180.0f, 34.0f))) {
      LeaveOnlineLobby();
    }
  } else if (onlineSession.IsOperationPending()) {
    ImGui::TextDisabled("EOS operation is pending.");
  } else {
    ImGui::InputInt("Max Members", &OnlineLobbyMaxMembers);
    if (onlineSession.CanShowLobbyActions() &&
        ImGui::Button("Create Online Lobby", ImVec2(180.0f, 34.0f))) {
      CreateOnlineLobby();
    }

    ImGui::InputInt("Search Max Results", &OnlineSearchMaxResults);
    if (onlineSession.CanShowSearchLobbies() &&
        ImGui::Button("Search Lobbies", ImVec2(180.0f, 34.0f))) {
      SearchOnlineLobbies();
    }

    if (!OnlineSearchResults.empty()) {
      ImGui::BeginChild("OnlineLobbySearchResults", ImVec2(0.0f, 130.0f), true);
      for (int index = 0; index < static_cast<int>(OnlineSearchResults.size()); ++index) {
        const FLobbyInfo& lobbyInfo = OnlineSearchResults[index];
        std::string label =
            std::to_string(index + 1) + ": " + lobbyInfo.GetStringAttribute("HOSTNAME", "Unknown") +
            " (" + std::to_string(lobbyInfo.CurrentMembers) + "/" +
            std::to_string(lobbyInfo.MaxMembers) +
            ") HostIP=" + (lobbyInfo.HostIPAddress.empty() ? "<none>" : lobbyInfo.HostIPAddress);
        if (ImGui::Selectable(label.c_str(), SelectedOnlineLobbyIndex == index)) {
          SelectedOnlineLobbyIndex = index;
        }
      }
      ImGui::EndChild();

      if (onlineSession.CanShowJoinLobby() &&
          ImGui::Button("Join Selected Lobby", ImVec2(180.0f, 34.0f))) {
        JoinSelectedOnlineLobby();
      }
    } else {
      ImGui::TextDisabled("No lobby search results.");
    }
  }

  ImGui::TextWrapped("%s", OnlineStatusMessage.c_str());

  ImGui::Separator();
  ImGui::TextUnformatted("Direct LAN Connection");
  if (ImGui::Button("Create Local Session", ImVec2(180.0f, 34.0f))) {
    StartHost();
  }

  ImGui::InputText("Server IP", ServerAddress, sizeof(ServerAddress));
  if (ImGui::Button("Join Local Session", ImVec2(180.0f, 34.0f))) {
    ConnectToHost();
  }

  ImGui::Separator();
  ImGui::TextWrapped("%s", StatusMessage.c_str());
  ImGui::End();
}

bool AMenuScene::StartHost() {
  SaveSettings();
  NetworkManager& network = NetworkManager::GetInstance();
  if (!network.StartServer(static_cast<uint16_t>(Port))) {
    StatusMessage = "Failed to start listen server.";
    return false;
  }

  GetWorld()->SetNetMode(ENetMode::ListenServer);
  if (!SceneManager::GetInstance().OpenLevelById(GameSceneIds::Lobby, ENetMode::ListenServer)) {
    StatusMessage = "Failed to open lobby scene.";
    return false;
  }
  StatusMessage = "Listen server started.";
  return true;
}

bool AMenuScene::ConnectToHost() {
  SaveSettings();
  NetworkManager& network = NetworkManager::GetInstance();
  if (!network.ConnectToServer(ServerAddress, static_cast<uint16_t>(Port))) {
    StatusMessage = "Failed to start client connection.";
    return false;
  }

  GetWorld()->SetNetMode(ENetMode::Client);
  StatusMessage = "Connecting. Waiting for server travel...";
  return true;
}

void AMenuScene::LoginWithDeviceId() {
  SaveSettings();

  OnlineStatusMessage = "EOS Device ID login requested.";
  if (!OnlineSessionManager::Get().LoginWithDeviceId(PlayerName, [this](bool bSuccess) {
        if (bSuccess) {
          OnlineStatusMessage = "EOS Device ID login succeeded.";
          const std::string localUserId = OnlineSessionManager::Get().GetLocalUserIdString();
          if (!localUserId.empty()) {
            OnlineStatusMessage += " ProductUserId: " + localUserId;
          }
        } else {
          OnlineStatusMessage = "EOS Device ID login failed. See Logs for details.";
        }
      })) {
    OnlineStatusMessage = "EOS Device ID login request was rejected.";
  }
}

void AMenuScene::CreateOnlineLobby() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before creating a lobby.";
    return;
  }

  if (OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Already in a lobby. Leave it before creating another one.";
    return;
  }

  SaveSettings();

  const std::string localIPAddress = NetworkUtils::GetLocalIPAddress();
  if (localIPAddress.empty()) {
    OnlineStatusMessage = "Local IP address was not found.";
    return;
  }

  FCreateLobbyRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxMembers = (std::max)(1, OnlineLobbyMaxMembers);
  request.bPublicAdvertised = true;
  request.HostIPAddress = localIPAddress;
  request.Attributes.push_back({"HOSTNAME", FLobbyAttributeValue::FromString(PlayerName), true});

  OnlineStatusMessage = "Create online lobby requested. HostIP=" + localIPAddress;
  if (!OnlineSessionManager::Get().CreateLobby(
          request, [this](bool bSuccess, const FLobbyInfo& lobbyInfo) {
            if (!bSuccess) {
              OnlineStatusMessage = "Create online lobby failed. See Logs for details.";
              return;
            }

            OnlineSearchResults.clear();
            SelectedOnlineLobbyIndex = -1;
            if (StartHost()) {
              OnlineStatusMessage = "Online lobby created: " + lobbyInfo.LobbyId +
                                    " HostIP=" + lobbyInfo.HostIPAddress;
              return;
            }

            OnlineStatusMessage = "Online lobby created, but listen server failed to start.";
            OnlineSessionManager::Get().LeaveLobby([this](bool bLeaveSuccess) {
              if (!bLeaveSuccess) {
                OnlineStatusMessage += " Leave lobby also failed. See Logs for details.";
              }
            });
          }
      )) {
    OnlineStatusMessage = "Create online lobby request was rejected.";
  }
}

void AMenuScene::SearchOnlineLobbies() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before searching lobbies.";
    return;
  }

  FLobbySearchRequest request;
  request.BucketId = GameLobbyBucketId;
  request.MaxResults = (std::max)(1, OnlineSearchMaxResults);

  OnlineSearchResults.clear();
  SelectedOnlineLobbyIndex = -1;
  OnlineStatusMessage = "Search lobbies requested.";
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
          }
      )) {
    OnlineStatusMessage = "Lobby search request was rejected.";
  }
}

void AMenuScene::JoinSelectedOnlineLobby() {
  if (!OnlineSessionManager::Get().IsLoggedIn()) {
    OnlineStatusMessage = "Login before joining a lobby.";
    return;
  }

  if (OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Already in a lobby. Leave it before joining another one.";
    return;
  }

  if (SelectedOnlineLobbyIndex < 0 ||
      SelectedOnlineLobbyIndex >= static_cast<int>(OnlineSearchResults.size())) {
    OnlineStatusMessage = "Select a lobby search result first.";
    return;
  }

  const FLobbyInfo lobbyInfo = OnlineSearchResults[SelectedOnlineLobbyIndex];
  if (lobbyInfo.HostIPAddress.empty()) {
    OnlineStatusMessage = "Selected lobby does not have HostIP.";
    return;
  }

  SaveSettings();

  OnlineStatusMessage = "Join lobby requested: " + lobbyInfo.LobbyId;
  if (!OnlineSessionManager::Get().JoinLobby(
          lobbyInfo, static_cast<uint16_t>(Port), [this, lobbyInfo](bool bSuccess) {
            if (!bSuccess) {
              OnlineStatusMessage =
                  "Join lobby or ENet connection failed. Search again if the result is stale.";
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
          }
      )) {
    OnlineStatusMessage = "Join lobby request was rejected.";
  }
}

void AMenuScene::LeaveOnlineLobby() {
  if (!OnlineSessionManager::Get().IsInLobby()) {
    OnlineStatusMessage = "Not in a lobby.";
    return;
  }

  const std::string lobbyId = OnlineSessionManager::Get().GetCurrentLobbyId();
  OnlineStatusMessage = "Leave lobby requested: " + lobbyId;
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
      })) {
    OnlineStatusMessage = "Leave lobby request was rejected.";
  }
}

void AMenuScene::LoadSettings() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    const std::string name = gi->player_name.empty() ? gi->user_id : gi->player_name;
    if (!name.empty()) {
      strncpy_s(PlayerName, sizeof(PlayerName), name.c_str(), _TRUNCATE);
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
