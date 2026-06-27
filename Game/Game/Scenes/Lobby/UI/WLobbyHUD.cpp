#include "Scenes/Lobby/UI/WLobbyHUD.h"

#include "Core/GI_main.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "World.h"

#include <algorithm>
#include <cstring>
#include <imgui.h>

void WLobbyHUD::SetLobbyController(PC_Lobby* InLobbyController)
{
	LobbyController = InLobbyController;
}

void WLobbyHUD::Draw()
{
	AWidgetBase::Draw();

	if (!LobbyController || !LobbyController->GetWorld()) {
		return;
	}

	const auto states = LobbyController->GetPlayerStates();
	ALobbyPlayerState* localState = LobbyController->FindLocalPlayerState();
	ALobbyPlayerState* hostState = LobbyController->FindHostPlayerState();
	if (hostState) {
		LobbyController->ApplyHostLobbyOptions();
	}

	World* world = LobbyController->GetWorld();
	ImGui::SetNextWindowSize(ImVec2(620.0f, 460.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Lobby");
	ImGui::Text("Mode: %s", world->IsListenServer() ? "Host" : (world->IsClient() ? "Client" : "Standalone"));
	ImGui::Text("Players: %d / %d", static_cast<int>(states.size()), LobbyController->GetMaxPlayers());
	ImGui::Separator();

	if (localState) {
		if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
			if (!gi->player_name.empty() && localState->GetPlayerName() != gi->player_name) {
				localState->SetPlayerName(gi->player_name);
			}
		}

		char nameBuffer[64] = {};
		strncpy_s(nameBuffer, sizeof(nameBuffer), localState->GetPlayerName().c_str(), _TRUNCATE);
		if (ImGui::InputText("My Name", nameBuffer, sizeof(nameBuffer))) {
			localState->SetPlayerName(nameBuffer);
			if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
				gi->player_name = nameBuffer;
			}
		}

		bool ready = localState->IsReady();
		if (ImGui::Checkbox("Ready", &ready)) {
			localState->SetReady(ready);
		}
	}
	else {
		ImGui::TextUnformatted("Waiting for player state...");
	}

	ImGui::Separator();
	ImGui::TextUnformatted("Participants");
	for (const auto* state : states) {
		if (!state) {
			continue;
		}

		ImGui::BulletText("[%u] %s  %s  Time: %.2f",
			state->OwnerConnectionId,
			state->GetPlayerName().c_str(),
			state->IsReady() ? "Ready" : "Not Ready",
			state->GetFinishTime());
	}

	ImGui::Separator();
	ImGui::Text("Selected Map: GameScene01");

	if (world->IsServer()) {
		int maxPlayers = LobbyController->GetMaxPlayers();
		if (ImGui::SliderInt("Max Players", &maxPlayers, 1, 8)) {
			LobbyController->SetMaxPlayers(maxPlayers);
		}

		const bool enoughPlayers = !states.empty();
		const bool allReady = std::all_of(states.begin(), states.end(), [](const ALobbyPlayerState* state) {
			return state && (state->OwnerConnectionId == 0 || state->IsReady());
		});
		if (!allReady) {
			ImGui::TextUnformatted("Waiting for clients to ready up.");
		}
		if (ImGui::Button("Start Game", ImVec2(180.0f, 34.0f)) && enoughPlayers && allReady) {
			LobbyController->StartGame();
		}
	}
	else {
		ImGui::TextUnformatted("Waiting for host to start the game.");
	}

	ImGui::End();
}