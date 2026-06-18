#include "ClearScene.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
#include "GI_main.h"
#include "UI/WClearHUD.h"
#include "UI/WNameSelectDialog.h"
#include "UI/WNameInputDialog.h"
#include "UI/WOverwriteConfirmDialog.h"
#include "LeaderBoardManager.h"
#include "UIManager.h"
#include "World.h"
#include "SpriteComponent.h"
#include <string>
#include <Pawn.h>
AClearScene::AClearScene()
{
	auto subText = std::make_unique<MSpriteComponent>(100, RenderSpace::Screen);
	subText->SubmitText("Press R to Return", 0xFFFFFF, -1, 255);
	subText->SetRelativeLocation({ 820.0f, 800.0f });
	AddComponent(std::move(subText));
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

	// Fetch and display leaderboard initially
	FetchAndDisplay();
	
	// Start name registration flow
	ShowNameFlow();
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
				// Proceed without registration
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

void AClearScene::OnUpdate(float DeltaTime)
{
	// Block R key scene transition if any dialog is active
	bool bIsDialogActive = (m_NameSelectDialog != nullptr || m_NameInputDialog != nullptr || m_OverwriteDialog != nullptr);
	if (bIsDialogActive) {
		return;
	}

	auto* kb = InputManager::GetInstance().GetDevice<KeyboardDevice>();
	if (kb && kb->GetPressStart(KEY_INPUT_R)) {
		SceneManager::GetInstance().OpenScene<AGameScene01>();
	}
}