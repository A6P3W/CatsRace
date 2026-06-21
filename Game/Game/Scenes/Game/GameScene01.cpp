#include "GameScene01.h"
#include <string>
#include "ObjectManager.h"
#include "Scenes/Game/Player.h"
#include "Objects/SampleA.h"
#include "ObjectManager.h"
#include "UMath.h"
#include "Log.h"
#include <PlayerController.h>
#include "PC_Game.h"
#include <Pawn.h>
#include "Goal.h"
#include <CircleCollisionComponent.h>
#include <RectangleCollisionComponent.h>
#include "Objects/Items/SpeedUpItem.h"
#include "ResourceManager.h"
#include <TimerHandle.h>
#include <TimerManager.h>
#include "Objects/Items/SpeedDown.h"
#include <SoundManager.h>
#include <LevelSerializer.h>
#include <SceneManager.h>
#include "Scenes/Clear/ClearScene.h"
#include <UIManager.h>
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "Scenes/Title/TitleScene.h"
#include <EnhancedInputComponent.h>
#include "Core/GI_main.h"
#include "Ghost/GhostRecorderComponent.h"
#include "Ghost/GhostData.h"
#include "Ghost/GhostPlaybackComponent.h"
#include "Ghost/GhostPlayer.h"
#include "Services/LeaderBoardManager.h"
AGameScene01::AGameScene01()
{
}

void AGameScene01::OnUpdate(float DeltaTime)
{
	if (RaceRunning) {
		RaceTime += DeltaTime;
		if (m_MainHUD) {
			m_MainHUD->UpdateTimerText(RaceTime);
		}
		if (m_GhostPlayer && m_GhostPlayer->GetPlaybackComponent()) {
			m_GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
		}
	}
}

void AGameScene01::BeginPlay()
{
	//auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
	//LBM->FetchLeaderBoard("b", [](const std::vector<FLeaderBoardEntry>& entries){
	//	for (const auto& entry : entries) {
	//		M_LOG("User: {}, Score: {}", entry.user_id, entry.score);
	//	}
	//	});
	LevelSerializer::Load(GetWorld(), "GameScene01.BLevel");

	SpawnPlayer<APlayer, PC_Game>(FVector2D{ -2800, -1700 }, 0);
	if (auto* player = dynamic_cast<APlayer*>(GetPlayerPawn())) {
		auto recorder = std::make_unique<MGhostRecorderComponent>();
		m_GhostRecorder = recorder.get();
		player->AddComponent(std::move(recorder));
	}

	SpawnActor<ASampleA>();
	LoadTopGhost();
	M_LOG("Default scene initialized", 0);


	m_MainHUD = SpawnActor<WMainHUD>();
	UIManager::GetInstance()->AddWidget(m_MainHUD);


	m_CountDownWidget = SpawnActor<WCountDown>();
	m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
	UIManager::GetInstance()->AddWidget(m_CountDownWidget);

	if (auto* pc = GetPlayerController()) {
		if (auto* inputComp = pc->GetInputComponent()) {
			inputComp->BindAction(InputAction::Pause, ETriggerEvent::Started, this, &AGameScene01::TogglePause);
		}
	}

	GetPlayerController()->SetInputMode(EInputMode::UIOnly);

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::RaceCountDown, 1.0f, true, 1.0f);
}

void AGameScene01::LoadTopGhost()
{
	auto* lbm = GetWorld()->SpawnActor<LeaderBoardManager>();
	lbm->FetchLeaderBoard("GameScene01", [this, lbm](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
		if (!bSuccess || entries.empty()) {
			M_LOG("Top ghost skipped: leaderboard is empty or unavailable");
			return;
		}

		const std::string topUserId = entries.front().user_id;
		lbm->FetchGhostData(topUserId, [this, topUserId](bool bGhostSuccess, const std::string& ghostData) {
			if (!bGhostSuccess || ghostData.empty()) {
				M_LOG("Top ghost skipped: ghost data is empty for {}", topUserId);
				return;
			}

			auto frames = GhostDataSerializer::Deserialize(ghostData);
			if (frames.empty()) {
				M_LOG("Top ghost skipped: failed to parse ghost data for {}", topUserId);
				return;
			}

			if (!m_GhostPlayer) {
				m_GhostPlayer = GetWorld()->SpawnActor<AGhostPlayer>();
			}
			m_GhostPlayer->SetGhostData(frames);
			if (m_GhostPlayer->GetPlaybackComponent()) {
				m_GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
			}
			M_LOG("Top ghost loaded: {} frames from {}", frames.size(), topUserId);
		});
	});
}
void AGameScene01::RaceFinish()
{
	RaceRunning = false;
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (m_GhostRecorder) {
		m_GhostRecorder->StopRecording();
	}
	if (gi) {
		gi->ClearTime = RaceTime;
		gi->map_id = "GameScene01";
		gi->LastGhostData = m_GhostRecorder ? m_GhostRecorder->GetSerializedData() : "";
	}
	SceneManager::GetInstance().OpenScene<AClearScene>();
}

void AGameScene01::RaceCountDown()
{
	m_CountDown--;

	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		m_CountDownWidget->SetCountText("Go!");
		RaceStart();
	}
	else {
		m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
		M_LOG(std::to_string(m_CountDown), 0);
		GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);
	}
}

void AGameScene01::RaceStart()
{
	M_LOG("start", 0);

	RaceRunning = true;
	if (m_GhostRecorder) {
		m_GhostRecorder->StartRecording();
	}

	GetPlayerController()->SetInputMode(EInputMode::GameOnly);
	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDown, 1.0f, false, 1.0f);
}

void AGameScene01::ClearCountDown()
{
	UIManager::GetInstance()->RemoveWidget(m_CountDownWidget);
	m_CountDownWidget = nullptr;
}

void AGameScene01::TogglePause()
{
	if (!RaceRunning && !bPaused) {
		return;
	}

	if (!bPaused) {
		bPaused = true;
		GetWorld()->SetSimulating(false);

		m_PauseMenu = GetWorld()->SpawnActor<WPauseMenu>();
		m_PauseMenu->OnResumePressed = [this]() {
			TogglePause();
		};
		m_PauseMenu->OnRestartPressed = [this]() {
			RestartGame();
		};
		m_PauseMenu->OnTitlePressed = [this]() {
			ReturnToTitle();
		};

		UIManager::GetInstance()->AddWidget(m_PauseMenu);
		UIManager::GetInstance()->SetFocusedWidget(m_PauseMenu);

		GetPlayerController()->SetInputMode(EInputMode::UIOnly);
	}
	else {
		bPaused = false;
		GetWorld()->SetSimulating(true);

		if (m_PauseMenu) {
			UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
			m_PauseMenu = nullptr;
		}

		GetPlayerController()->SetInputMode(EInputMode::GameOnly);
	}
}

void AGameScene01::RestartGame()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	SceneManager::GetInstance().OpenScene<AGameScene01>();
}

void AGameScene01::ReturnToTitle()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	SceneManager::GetInstance().OpenScene<ATitleScene>();
}
