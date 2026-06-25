#include "ClearScene.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Title/TitleScene.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "UI/WClearHUD.h"
#include "UI/WNameSelectDialog.h"
#include "UI/WNameInputDialog.h"
#include "UI/WOverwriteConfirmDialog.h"
#include "UI/WPostGameDialog.h"
#include "Services/LeaderBoardManager.h"
#include "UIManager.h"
#include "World.h"
#include "SpriteComponent.h"
#include <string>
#include <algorithm>
#include <imgui.h>
#include <Pawn.h>
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Game/Player.h"
AClearScene::AClearScene()
{
}

void AClearScene::BeginPlay()
{
	AGameModeBase::BeginPlay();
	SpawnPlayer<APawn, APlayerController>();
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	float clearTime = gi ? gi->ClearTime : 0.0f;

	m_ClearHUD = GetWorld()->SpawnActor<WClearHUD>();
	UIManager::GetInstance()->AddWidget(m_ClearHUD);

	if (m_ClearHUD) {
		m_ClearHUD->SetClearTime(clearTime);
	}

	if (GetWorld()->IsServer()) {
		SpawnResultStatesFromGameInstance();
	}

	if (GetWorld()->IsStandalone()) {
		// Fetch and display leaderboard initially
		FetchAndDisplay();
		
		// Start name registration flow
		ShowNameFlow();
	}
}

void AClearScene::ShowNameFlow()
{
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (!gi) return;

	if (!gi->user_id.empty()) {
		m_NameSelectDialog = GetWorld()->SpawnActor<WNameSelectDialog>();
		m_NameSelectDialog->SetZOrderOffset(10);
		m_NameSelectDialog->SetExistingName(gi->user_id);
		m_NameSelectDialog->SetOnResult([this, gi](ENameSelectResult result) {
			if (m_NameSelectDialog) {
				m_NameSelectDialog->Destroy();
				m_NameSelectDialog = nullptr;
			}
			if (result == ENameSelectResult::UseExisting) {
				ExecutePostScore(gi->user_id);
			} else if (result == ENameSelectResult::EnterNew) {
				ShowNameInputDialog();
			} else if (result == ENameSelectResult::Skip) {
				ShowPostGameDialog();
			}
		});
		UIManager::GetInstance()->AddWidget(m_NameSelectDialog);
		UIManager::GetInstance()->SetFocusedWidget(m_NameSelectDialog);
	} else {
		ShowNameInputDialog();
	}
}

void AClearScene::ShowNameInputDialog()
{
	m_NameInputDialog = GetWorld()->SpawnActor<WNameInputDialog>();
	m_NameInputDialog->SetZOrderOffset(10);
	m_NameInputDialog->SetOnNameConfirmed([this](const std::string& name) {
		if (m_NameInputDialog) {
			m_NameInputDialog->Destroy();
			m_NameInputDialog = nullptr;
		}
		CheckDuplicateAndPost(name);
	});
	m_NameInputDialog->SetOnCancelled([this]() {
		if (m_NameInputDialog) {
			m_NameInputDialog->Destroy();
			m_NameInputDialog = nullptr;
		}
		ShowPostGameDialog();
	});
	UIManager::GetInstance()->AddWidget(m_NameInputDialog);
	UIManager::GetInstance()->SetFocusedWidget(m_NameInputDialog);
}

void AClearScene::CheckDuplicateAndPost(const std::string& name)
{
	bool isDuplicate = false;
	for (const auto& existingId : m_FetchedUserIds) {
		if (existingId == name) {
			isDuplicate = true;
			break;
		}
	}

	if (isDuplicate) {
		m_OverwriteDialog = GetWorld()->SpawnActor<WOverwriteConfirmDialog>();
		m_OverwriteDialog->SetZOrderOffset(10);
		m_OverwriteDialog->SetOnResult([this, name](EOverwriteResult result) {
			if (m_OverwriteDialog) {
				m_OverwriteDialog->Destroy();
				m_OverwriteDialog = nullptr;
			}
			if (result == EOverwriteResult::Overwrite) {
				if (auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
					gi->user_id = name;
				}
				ExecutePostScore(name);
			} else if (result == EOverwriteResult::ReEnter) {
				ShowNameInputDialog();
			}
		});
		UIManager::GetInstance()->AddWidget(m_OverwriteDialog);
		UIManager::GetInstance()->SetFocusedWidget(m_OverwriteDialog);
	} else {
		if (auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
			gi->user_id = name;
		}
		ExecutePostScore(name);
	}
}

void AClearScene::ExecutePostScore(const std::string& name)
{
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (!gi) return;

	std::string map_id = gi->map_id;
	auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
	LBM->PostScore(map_id, name, [this](bool bSuccess) {
		if (!bSuccess) {
			if (m_ClearHUD) {
				m_ClearHUD->SetErrorText("Failed to Post Score");
			}
		}
		FetchAndDisplay();
		ShowPostGameDialog();
	});
}

void AClearScene::FetchAndDisplay()
{
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (!gi) return;

	std::string map_id = gi->map_id;
	auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
	LBM->FetchLeaderBoard(map_id, [this](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
		if (!bSuccess) {
			if (m_ClearHUD) {
				m_ClearHUD->SetErrorText("Failed to Fetch LeaderBoard");
			}
			return;
		}
		m_FetchedUserIds.clear();
		for (const auto& entry : entries) {
			m_FetchedUserIds.push_back(entry.user_id);
		}
		if (m_ClearHUD) {
			m_ClearHUD->SetLeaderBoard(entries);
		}
	});
}

void AClearScene::ShowPostGameDialog()
{
	m_PostGameDialog = GetWorld()->SpawnActor<WPostGameDialog>();
	m_PostGameDialog->SetZOrderOffset(10);
	m_PostGameDialog->SetOnResult([this](EPostGameResult result) {
		if (m_PostGameDialog) {
			m_PostGameDialog->Destroy();
			m_PostGameDialog = nullptr;
		}
		if (result == EPostGameResult::PlayAgain) {
			GetWorld()->ServerTravel(GameSceneIds::Game01);
		} else if (result == EPostGameResult::BackToTitle) {
			GetWorld()->ServerTravel(GameSceneIds::Lobby);
		}
	});
	UIManager::GetInstance()->AddWidget(m_PostGameDialog);
	UIManager::GetInstance()->SetFocusedWidget(m_PostGameDialog);
}

void AClearScene::SpawnResultStatesFromGameInstance()
{
	auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (!gi) {
		return;
	}

	if (gi->multiplayer_results.empty()) {
		GI_main::FMultiplayerResult result;
		result.ConnectionId = 0;
		result.PlayerName = gi->player_name.empty() ? gi->user_id : gi->player_name;
		result.bFinished = true;
		result.FinishTime = gi->ClearTime;
		gi->multiplayer_results.push_back(result);
	}

	for (const auto& result : gi->multiplayer_results) {
		auto* state = GetWorld()->SpawnActor<ALobbyPlayerState>();
		state->OwnerConnectionId = result.ConnectionId;
		state->bReplicates = true;
		state->bHasAuthority = true;
		state->bIsLocallyControlled = result.ConnectionId == 0;
		state->SetPlayerName(result.PlayerName);
		state->SetFinishResult(result.bFinished, result.FinishTime);
	}
}

std::vector<ALobbyPlayerState*> AClearScene::GetResultStates()
{
	std::vector<ALobbyPlayerState*> states;
	if (!GetWorld() || !GetWorld()->GetObjectManager()) {
		return states;
	}
	for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
		if (auto* state = dynamic_cast<ALobbyPlayerState*>(actorPtr.get())) {
			if (!state->IsPendingDestroy()) {
				states.push_back(state);
			}
		}
	}
	std::sort(states.begin(), states.end(), [](const ALobbyPlayerState* a, const ALobbyPlayerState* b) {
		if (a->IsFinished() != b->IsFinished()) return a->IsFinished() > b->IsFinished();
		if (a->GetFinishTime() != b->GetFinishTime()) return a->GetFinishTime() < b->GetFinishTime();
		return a->OwnerConnectionId < b->OwnerConnectionId;
	});
	return states;
}

void AClearScene::Draw()
{
	AGameModeBase::Draw();
	if (GetWorld()->IsStandalone()) {
		return;
	}

	const auto states = GetResultStates();
	ImGui::SetNextWindowSize(ImVec2(620.0f, 460.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Multiplayer Results");
	ImGui::TextUnformatted("Race Results");
	ImGui::Separator();
	int rank = 1;
	for (const auto* state : states) {
		if (!state) continue;
		if (state->IsFinished()) {
			ImGui::Text("%d. %s  %.2f", rank++, state->GetPlayerName().c_str(), state->GetFinishTime());
		}
		else {
			ImGui::Text("-. %s  DNF", state->GetPlayerName().c_str());
		}
	}
	ImGui::Separator();
	if (GetWorld()->IsServer()) {
		if (ImGui::Button("Replay", ImVec2(160.0f, 34.0f))) {
			GetWorld()->ServerTravel(GameSceneIds::Game01);
		}
		ImGui::SameLine();
		if (ImGui::Button("Back To Lobby", ImVec2(160.0f, 34.0f))) {
			GetWorld()->ServerTravel(GameSceneIds::Lobby);
		}
	}
	else {
		ImGui::TextUnformatted("Waiting for host.");
	}
	ImGui::End();
}
void AClearScene::OnUpdate(float DeltaTime)
{
	(void)DeltaTime;
	if (!GetWorld() || !GetWorld()->GetObjectManager()) {
		return;
	}
	for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
		if (auto* player = dynamic_cast<APlayer*>(actorPtr.get())) {
			player->Destroy();
		}
	}
}
