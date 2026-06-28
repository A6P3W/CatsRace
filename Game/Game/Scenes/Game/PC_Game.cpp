#include "PC_Game.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "KeyboardDevice.h"
#include "GamePadDevice.h"
#include "EnhancedInputComponent.h"
#include "World.h"
#include <DxLib.h>

#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "UIManager.h"
#include "SoundManager.h"
#include "Scenes/Game/GameSceneBase.h"
#include "SceneManager.h"
#include "Core/GameSceneIds.h"
#include "NetworkManager.h"

REGISTER_ACTOR(PC_Game)

PC_Game::PC_Game()
{
	SetUpdateableAnytime(true);
}

void PC_Game::BeginPlay()
{
	APlayerController::BeginPlay();

	if (bIsLocallyControlled) {
		m_MainHUD = GetWorld()->SpawnActor<WMainHUD>();
		UIManager::GetInstance()->AddWidget(m_MainHUD);

		m_CountDownWidget = GetWorld()->SpawnActor<WCountDown>();
		m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
		UIManager::GetInstance()->AddWidget(m_CountDownWidget);

		if (auto* inputComp = GetInputComponent()) {
			inputComp->BindAction(InputAction::Pause, ETriggerEvent::Started, this, &PC_Game::TogglePause);
		}
		SetInputMode(EInputMode::UIOnly);

		GetWorldTimerManager().SetTimer(CountHandle, this, &PC_Game::RaceCountDown, 1.0f, true, 1.0f);
	}
}

void PC_Game::OnUpdate(float DeltaTime)
{
	APlayerController::OnUpdate(DeltaTime);

	if (bIsLocallyControlled && RaceRunning) {
		RaceTime += DeltaTime;
		if (m_MainHUD) {
			m_MainHUD->UpdateTimerText(RaceTime);
		}
	}
}

void PC_Game::RaceCountDown()
{
	m_CountDown--;

	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		if (m_CountDownWidget) {
			m_CountDownWidget->SetCountText("Go!");
		}
		RaceRunning = true;
		SetInputMode(EInputMode::GameOnly);
		GetWorldTimerManager().SetTimer(CountHandle, this, &PC_Game::ClearCountDown, 1.0f, false, 1.0f);
	}
	else {
		if (m_CountDownWidget) {
			m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
		}
		GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);
	}
}

void PC_Game::ClearCountDown()
{
	if (m_CountDownWidget) {
		UIManager::GetInstance()->RemoveWidget(m_CountDownWidget);
		m_CountDownWidget = nullptr;
	}
}

void PC_Game::TogglePause()
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

		SetInputMode(EInputMode::UIOnly);
	}
	else {
		bPaused = false;
		GetWorld()->SetSimulating(true);

		if (m_PauseMenu) {
			UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
			m_PauseMenu = nullptr;
		}

		SetInputMode(EInputMode::GameOnly);
	}
}

void PC_Game::RestartGame()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	if (GetWorld()->IsServer()) {
		if (auto* gameMode = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
			gameMode->RestartGame();
		}
	}
}

void PC_Game::ReturnToTitle()
{
	bPaused = false;
	GetWorld()->SetSimulating(true);

	if (m_PauseMenu) {
		UIManager::GetInstance()->RemoveWidget(m_PauseMenu);
		m_PauseMenu = nullptr;
	}

	if (GetWorld()->IsServer()) {
		if (auto* gameMode = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
			gameMode->ReturnToTitle();
		}
	}
	else {
		NetworkManager::GetInstance().Disconnect();
		SceneManager::GetInstance().OpenSceneById(GameSceneIds::Title);
	}
}

void PC_Game::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent)
{
	APlayerController::SetupPlayerInputComponent(PlayerInputComponent);
}

void PC_Game::SetupInputMappings()
{
	APlayerController::SetupInputMappings();

	auto& IM = InputManager::GetInstance();
	auto* kb = IM.GetDevice<KeyboardDevice>();
	auto* pad = IM.GetDevice<GamepadDevice>();
	auto* Mapper = GetInputMapper();

	Mapper->RemoveMapping(InputActionLower::MoveY);
	Mapper->RemoveMapping(InputAction::Interact);

	if (kb) {
		Mapper->AddMapping(InputActionLower::MoveY, kb, KEY_INPUT_W, "", 1.0f);
		Mapper->AddMapping(InputActionLower::MoveY, kb, KEY_INPUT_S, "", -1.0f);
		Mapper->AddMapping(InputAction::Interact, kb, KEY_INPUT_F);
		Mapper->AddMapping("DRIFT", kb, KEY_INPUT_SPACE);
	}
	if (pad) {
		// ゲームプレイ中の MoveY には左スティックをバインドせず、トリガーのみで操作させる
		Mapper->AddAxisMapping(InputActionLower::MoveY, pad, static_cast<int>(AxisID::RightTrigger), 1.0f);
		Mapper->AddAxisMapping(InputActionLower::MoveY, pad, static_cast<int>(AxisID::LeftTrigger), -1.0f);

		Mapper->AddMapping(InputAction::Interact, pad, PAD_INPUT_4);
		Mapper->AddMapping("DRIFT", pad, PAD_INPUT_1);
	}
}