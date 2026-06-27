#include "Scenes/Menu/MenuScene.h"

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "Scenes/Common/ImGuiHelper.h"
#include "World.h"

#include <algorithm>
#include <cstring>
#include <imgui.h>

REGISTER_GAME_MODE(AMenuScene)

AMenuScene::AMenuScene()
{
	SetUpdateableAnytime(true);
}

void AMenuScene::BeginPlay()
{
	AGameModeBase::BeginPlay();
	LoadSettings();
}

void AMenuScene::OnUpdate(float DeltaTime)
{
	(void)DeltaTime;
}

void AMenuScene::Draw()
{
	AGameModeBase::Draw();

	ImGui::SetNextWindowSize(ImVec2(520.0f, 360.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Local Multiplayer Menu");
	ImGuiHelper::DrawCenteredStatusText("Cats Race");
	ImGui::Separator();

	ImGui::InputText("Player Name", PlayerName, sizeof(PlayerName));
	ImGui::InputInt("Port", &Port);
	Port = std::clamp(Port, 1, 65535);

	if (ImGui::Button("Create Session", ImVec2(180.0f, 34.0f))) {
		StartHost();
	}

	ImGui::Separator();
	ImGui::InputText("Server IP", ServerAddress, sizeof(ServerAddress));
	if (ImGui::Button("Join Session", ImVec2(180.0f, 34.0f))) {
		ConnectToHost();
	}

	ImGui::Separator();
	ImGui::TextWrapped("%s", StatusMessage.c_str());
	ImGui::End();
}

void AMenuScene::StartHost()
{
	SaveSettings();
	NetworkManager& network = NetworkManager::GetInstance();
	if (!network.StartServer(static_cast<uint16_t>(Port))) {
		StatusMessage = "Failed to start listen server.";
		return;
	}

	GetWorld()->SetNetMode(ENetMode::ListenServer);
	if (!SceneManager::GetInstance().OpenSceneById(GameSceneIds::Lobby, ENetMode::ListenServer)) {
		StatusMessage = "Failed to open lobby scene.";
		return;
	}
	StatusMessage = "Listen server started.";
}

void AMenuScene::ConnectToHost()
{
	SaveSettings();
	NetworkManager& network = NetworkManager::GetInstance();
	if (!network.ConnectToServer(ServerAddress, static_cast<uint16_t>(Port))) {
		StatusMessage = "Failed to start client connection.";
		return;
	}

	GetWorld()->SetNetMode(ENetMode::Client);
	StatusMessage = "Connecting. Waiting for server travel...";
}

void AMenuScene::LoadSettings()
{
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

void AMenuScene::SaveSettings()
{
	if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
		gi->player_name = PlayerName[0] == '\0' ? "Player" : PlayerName;
		gi->last_server_ip = ServerAddress;
	}
}