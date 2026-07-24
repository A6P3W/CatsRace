#include "PC_Game.h"

#include <DxLib.h>

#include "Core/GameSceneIds.h"
#include "EnhancedInputComponent.h"
#include "GamePadDevice.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "KeyboardDevice.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "Scenes/Common/UI/WControlGuide.h"
#include "Scenes/Game/GameScene2.h"
#include "Scenes/Game/GameSceneBase.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "Scenes/Practice/PracticeGameMode.h"
#include "SoundManager.h"
#include "UIManager.h"
#include "World.h"
REGISTER_ACTOR(PC_Game)

PC_Game::PC_Game() { SetUpdateableAnytime(true); }

void PC_Game::BeginPlay() {
  APlayerController::BeginPlay();

  if (bIsLocallyControlled) {
    if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
      gameScene->InitializeGhostForCurrentMap();
    }

    MainHUD = GetWorld()->SpawnActor<WMainHUD>();
    UIManager::GetInstance()->AddWidget(MainHUD);

    if (dynamic_cast<APracticeGameMode*>(GetWorld()->GetGameMode())) {
      RaceRunning = true;
      SetInputMode(EInputMode::GameOnly);
      if (auto* player = dynamic_cast<APlayer*>(GetPawn())) {
        player->SetCanMove(true);
      }
    } else {
      CountDownWidget = GetWorld()->SpawnActor<WCountDown>();
      CountDownWidget->SetCountText(std::to_string(m_CountDown));
      UIManager::GetInstance()->AddWidget(CountDownWidget);

      SetInputMode(EInputMode::UIOnly);

      GetWorldTimerManager().SetTimer(CountHandle, this, &PC_Game::RaceCountDown, 1.0f, true, 1.0f);
    }

    ControlGuideWidget = GetWorld()->SpawnActor<WControlGuide>();
    ControlGuideWidget->SetGuideMode(EControlGuideMode::Game);
    UIManager::GetInstance()->AddWidget(ControlGuideWidget);
  }
}

void PC_Game::OnUpdate(float DeltaTime) {
  APlayerController::OnUpdate(DeltaTime);

  if (bIsLocallyControlled) {
    if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
      gameScene->InitializeGhostForCurrentMap();
    }

    if (RaceRunning) {
      RaceTime += DeltaTime;
      if (MainHUD) {
        MainHUD->UpdateTimerText(RaceTime);
      }
    }
   
    if (MainHUD) {
      if (auto* player = dynamic_cast<APlayer*>(GetPawn())) {
        MainHUD->SetHeldItemVisible(player->HasHeldItem());

   
        MainHUD->SetLapVisible(true);
        MainHUD->UpdateLapText(player->GetCurrentLap(), 3);
      }
    }
  }
}
void PC_Game::RaceCountDown() {
  m_CountDown--;

  if (m_CountDown <= 0) {
    GetWorldTimerManager().ClearTimer(CountHandle);
    if (CountDownWidget) {
      CountDownWidget->SetCountText("Go!");
    }
    RaceRunning = true;
    if (auto* gameScene = dynamic_cast<AGameSceneBase*>(GetWorld()->GetGameMode())) {
      gameScene->StartLocalRace();
    }
    SetInputMode(EInputMode::GameOnly);
    GetWorldTimerManager().SetTimer(CountHandle, this, &PC_Game::ClearCountDown, 1.0f, false, 1.0f);
  } else {
    if (CountDownWidget) {
      CountDownWidget->SetCountText(std::to_string(m_CountDown));
    }
    GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);
  }
}

void PC_Game::ClearCountDown() {
  if (CountDownWidget) {
    UIManager::GetInstance()->RemoveWidget(CountDownWidget);
    CountDownWidget = nullptr;
  }
}

void PC_Game::TogglePause() {
  if (!RaceRunning && !bPaused) {
    return;
  }

  if (!bPaused) {
    bPaused = true;

    PauseMenu = GetWorld()->SpawnActor<WPauseMenu>();
    PauseMenu->OnResumePressed = [this]() { TogglePause(); };
    PauseMenu->OnRestartPressed = [this]() { RestartGame(); };
    if (dynamic_cast<APracticeGameMode*>(GetWorld()->GetGameMode())) {
      PauseMenu->OnTitlePressed = [this]() { LeaveSession(); };
    } else {
      PauseMenu->OnTitlePressed = [this]() { ReturnToLobby(); };
    }
    PauseMenu->OnLeavePressed = [this]() { LeaveSession(); };

    UIManager::GetInstance()->AddWidget(PauseMenu);
    UIManager::GetInstance()->SetFocusedWidget(PauseMenu);

    SetInputMode(EInputMode::UIOnly);
    if (ControlGuideWidget) {
      ControlGuideWidget->SetGuideMode(EControlGuideMode::UI);
    }
  } else {
    bPaused = false;

    if (PauseMenu) {
      UIManager::GetInstance()->RemoveWidget(PauseMenu);
      PauseMenu = nullptr;
    }

    SetInputMode(EInputMode::GameOnly);
    if (ControlGuideWidget) {
      ControlGuideWidget->SetGuideMode(EControlGuideMode::Game);
    }
  }
}

void PC_Game::RestartGame() {
  auto& SM = SceneManager::GetInstance();
  GetWorld()->ServerTravel(SM.GetCurrentLevelPath());
}

void PC_Game::ReturnToLobby() { GetWorld()->ServerTravel(GameSceneIds::Lobby); }

void PC_Game::LeaveSession() {
  NetworkManager::GetInstance().Disconnect();
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu, ENetMode::Standalone);
}

void PC_Game::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) {
  APlayerController::SetupPlayerInputComponent(PlayerInputComponent);
  PlayerInputComponent->BindAction(
      InputAction::Pause, ETriggerEvent::Started, this, &PC_Game::TogglePause
  );
}

void PC_Game::SetupInputMappings() {
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
    Mapper->AddMapping(InputAction::Pause, kb, KEY_INPUT_ESCAPE);
    Mapper->AddMapping("DRIFT", kb, KEY_INPUT_SPACE);
    Mapper->AddMapping("USE_ITEM", kb, KEY_INPUT_E);
  }
  if (pad) {
    // ゲームプレイ中の MoveY には左スティックをバインドせず、トリガーのみで操作させる
    Mapper->AddAxisMapping(
        InputActionLower::MoveY, pad, static_cast<int>(AxisID::RightTrigger), 1.0f
    );
    Mapper->AddAxisMapping(
        InputActionLower::MoveY, pad, static_cast<int>(AxisID::LeftTrigger), -1.0f
    );

    Mapper->AddMapping(InputAction::Interact, pad, PAD_INPUT_4);
    Mapper->AddMapping("DRIFT", pad, PAD_INPUT_1);
    Mapper->AddMapping("USE_ITEM", pad, PAD_INPUT_3);
  }
}
