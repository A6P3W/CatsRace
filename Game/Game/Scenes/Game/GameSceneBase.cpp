#include "GameSceneBase.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "ObjectManager.h"
#include "Scenes/Game/Player.h"
#include "Objects/SampleA.h"
#include "UMath.h"
#include "Log.h"
#include <PlayerController.h>
#include "PC_Game.h"
#include "ResourceManager.h"
#include <TimerHandle.h>
#include <TimerManager.h>
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

AGameSceneBase::AGameSceneBase(std::string mapId, std::string levelFileName, FVector2D playerStartLocation)
	: MapId(std::move(mapId))
	, LevelFileName(std::move(levelFileName))
	, PlayerStartLocation(playerStartLocation)
{
}

void AGameSceneBase::OnUpdate(float DeltaTime)
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

void AGameSceneBase::BeginPlay()
{
	AGameModeBase::BeginPlay();

	LevelSerializer::Load(GetWorld(), LevelFileName);

	SpawnPlayer<APlayer, PC_Game>(PlayerStartLocation, 0);
	if (auto* player = dynamic_cast<APlayer*>(GetPlayerPawn())) {
		auto recorder = std::make_unique<MGhostRecorderComponent>();
		m_GhostRecorder = recorder.get();
		player->AddComponent(std::move(recorder));
	}

	SpawnActor<ASampleA>();
	LoadTopGhost();
	M_LOG("Game scene initialized: {}", MapId);

	m_MainHUD = SpawnActor<WMainHUD>();
	UIManager::GetInstance()->AddWidget(m_MainHUD);

	m_CountDownWidget = SpawnActor<WCountDown>();
	m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
	UIManager::GetInstance()->AddWidget(m_CountDownWidget);

	if (auto* pc = GetPlayerController()) {
		if (auto* inputComp = pc->GetInputComponent()) {
			inputComp->BindAction(InputAction::Pause, ETriggerEvent::Started, this, &AGameSceneBase::TogglePause);
		}
		pc->SetInputMode(EInputMode::UIOnly);
	}

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameSceneBase::RaceCountDown, 1.0f, true, 1.0f);
}

void AGameSceneBase::LoadTopGhost()
{
	auto* lbm = GetWorld()->SpawnActor<LeaderBoardManager>();
	const std::string mapId = MapId;
	lbm->FetchLeaderBoard(mapId, [this, lbm, mapId](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
		if (!bSuccess || entries.empty()) {
			M_LOG("Top ghost skipped: leaderboard is empty or unavailable");
			return;
		}

		const std::string topUserId = entries.front().user_id;
		lbm->FetchGhostData(mapId, { topUserId }, [this, topUserId](bool bGhostSuccess, const std::unordered_map<std::string, std::string>& ghostDataById) {
			auto ghostDataIt = ghostDataById.find(topUserId);
			if (!bGhostSuccess || ghostDataIt == ghostDataById.end() || ghostDataIt->second.empty()) {
				M_LOG("Top ghost skipped: ghost data is empty for {}", topUserId);
				return;
			}

			auto frames = GhostDataSerializer::Deserialize(ghostDataIt->second);
			if (frames.empty()) {
				M_LOG("Top ghost skipped: failed to parse ghost data for {}", topUserId);
				return;
			}

			if (!m_GhostPlayer) {
				m_GhostPlayer = GetWorld()->SpawnActor<AGhostPlayer>();
			}
			m_GhostPlayer->SetUserId(topUserId);
			m_GhostPlayer->SetGhostData(frames);
			if (m_GhostPlayer->GetPlaybackComponent()) {
				m_GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
			}
			M_LOG("Top ghost loaded: {} frames from {}", frames.size(), topUserId);
		});
	});
}

void AGameSceneBase::RaceFinish()
{
	RaceRunning = false;
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	if (m_GhostRecorder) {
		m_GhostRecorder->StopRecording();
	}
	if (gi) {
		gi->ClearTime = RaceTime;
		gi->map_id = MapId;
		gi->LastGhostData = m_GhostRecorder ? m_GhostRecorder->GetSerializedData() : "";
	}
	SceneManager::GetInstance().OpenScene<AClearScene>();
}

void AGameSceneBase::RaceCountDown()
{
	m_CountDown--;

	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		if (m_CountDownWidget) {
			m_CountDownWidget->SetCountText("Go!");
		}
		RaceStart();
	}
	else {
		if (m_CountDownWidget) {
			m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
		}
		M_LOG(std::to_string(m_CountDown), 0);
		GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);
	}
}

void AGameSceneBase::RaceStart()
{
	M_LOG("start", 0);

	RaceRunning = true;
	if (m_GhostRecorder) {
		m_GhostRecorder->StartRecording();
	}

	if (auto* pc = GetPlayerController()) {
		pc->SetInputMode(EInputMode::GameOnly);
	}
	if (auto* player = dynamic_cast<APlayer*>(GetPlayerPawn())) {
		player->SetCanMove(true);
	}

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameSceneBase::ClearCountDown, 1.0f, false, 1.0f);
}

void AGameSceneBase::ClearCountDown()
{
	if (m_CountDownWidget) {
		UIManager::GetInstance()->RemoveWidget(m_CountDownWidget);
		m_CountDownWidget = nullptr;
	}
}

void AGameSceneBase::TogglePause()
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

		if (auto* pc = GetPlayerController()) {
			pc->SetInputMode(EInputMode::UIOnly);
		}
	}
	else {
		bPaused = false;
		GetWorld()->SetSimulating(true);

		if (m_PauseMenu) {
			UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
			m_PauseMenu = nullptr;
		}

		if (auto* pc = GetPlayerController()) {
			pc->SetInputMode(EInputMode::GameOnly);
		}
	}
}

void AGameSceneBase::RestartGame()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	OpenCurrentScene();
}

void AGameSceneBase::ReturnToTitle()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	SceneManager::GetInstance().OpenScene<ATitleScene>();
}

