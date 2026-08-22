#include "PC_Game.h"

#include <cmath>

#include "Core/CatsRacePacketType.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "EnhancedInputComponent.h"
#include "GamePadDevice.h"
#include "GameScene01.h"
#include "Ghost/GhostData.h"
#include "Ghost/GhostPlaybackComponent.h"
#include "Ghost/GhostPlayer.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "KeyboardDevice.h"
#include "Log.h"
#include "NetPacketType.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "Scenes/Common/UI/WControlGuide.h"
#include "Scenes/Game/GameScene2.h"
#include "Scenes/Game/GameSceneBase.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Game/PlayerDirectionIndicator.h"
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "Scenes/Practice/PracticeGameMode.h"
#include "SoundManager.h"
#include "UIManager.h"
#include "World.h"

REGISTER_ACTOR(PC_Game)

PC_Game::PC_Game() { SetUpdateableAnytime(true); }

PC_Game::~PC_Game() {
  if (NetworkPacketCallbackHandle != 0) {
    NetworkManager::GetInstance().RemoveOnPacketReceived(NetworkPacketCallbackHandle);
  }
}

void PC_Game::BeginPlay() {
  APlayerController::BeginPlay();

  if (bIsLocallyControlled) {
    if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
      double PendingStartTime = 0.0;
      if (gi->ConsumePendingRaceStartTime(PendingStartTime)) {
        ReceiveRaceStartTime(PendingStartTime);
      }
    }

    NetworkPacketCallbackHandle = NetworkManager::GetInstance().AddOnPacketReceived(
        [this](FNetworkConnectionId ConnectionId, FNetBuffer& Buffer) {
          HandleNetworkPacket(ConnectionId, Buffer);
        }
    );

    MainHUD = GetWorld()->SpawnActor<WMainHUD>();
    UIManager::GetInstance()->AddWidget(MainHUD);
    PlayerDirectionIndicator = GetWorld()->SpawnActor<APlayerDirectionIndicator>();

    if (dynamic_cast<APracticeGameMode*>(GetWorld()->GetGameMode())) {
      RaceRunning = true;
      SetInputMode(EInputMode::GameOnly);
      if (auto* player = dynamic_cast<APlayer*>(GetPawn())) {
        player->SetCanMove(true);
        if (PlayerDirectionIndicator) {
          PlayerDirectionIndicator->InitializePlayers(player);
        }
      }
    } else {
      SpawnRaceGhosts();
      CountDownWidget = GetWorld()->SpawnActor<WCountDown>();
      UIManager::GetInstance()->AddWidget(CountDownWidget);

      SetInputMode(EInputMode::UIOnly);
    }

    ControlGuideWidget = GetWorld()->SpawnActor<WControlGuide>();
    ControlGuideWidget->SetGuideMode(EControlGuideMode::Game);
    UIManager::GetInstance()->AddWidget(ControlGuideWidget);
  }
}

void PC_Game::OnUpdate(float DeltaTime) {
  APlayerController::OnUpdate(DeltaTime);

  if (bIsLocallyControlled) {
    if (!RaceRunning && !bHasRaceStartTime) {
      if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
        double PendingStartTime = 0.0;
        if (gi->ConsumePendingRaceStartTime(PendingStartTime)) {
          ReceiveRaceStartTime(PendingStartTime);
        }
      }
    }

    if (!RaceRunning && bHasRaceStartTime) {
      const double RemainingTime =
          RaceStartServerTime - NetworkManager::GetInstance().GetEstimatedServerTime();
      if (RemainingTime <= 0.0) {
        StartLocalRace();
        return;
      } else {
        UpdateCountdown(RemainingTime);
      }
    }

    if (RaceRunning) {
      RaceTime += DeltaTime;
      for (AGhostPlayer* GhostPlayer : GhostPlayers) {
        if (GhostPlayer && GhostPlayer->GetPlaybackComponent()) {
          GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
        }
      }
      if (MainHUD) {
        MainHUD->UpdateTimerText(RaceTime);
      }
    }

    if (MainHUD) {
      if (auto* player = dynamic_cast<APlayer*>(GetPawn())) {
        MainHUD->SetHeldItemVisible(player->HasHeldItem());
        bool bIsStage1 = (dynamic_cast<AGameScene01*>(GetWorld()->GetGameMode()) != nullptr);

        if (bIsStage1) {
          MainHUD->SetLapVisible(false);
        } else {
          MainHUD->SetLapVisible(true);
          MainHUD->UpdateLapText(player->GetCurrentLap(), 2);
        }
      }
    }
  }
}

void PC_Game::SpawnRaceGhosts() {
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance) {
    return;
  }

  for (const FRaceGhostData& Ghost : GameInstance->RaceGhosts) {
    const auto Frames = GhostDataSerializer::Deserialize(Ghost.GhostData);
    if (Frames.empty()) {
      M_LOG(Warning, "Race ghost skipped: failed to parse data for {}", Ghost.UserId);
      continue;
    }

    auto* GhostPlayer = GetWorld()->SpawnActor<AGhostPlayer>();
    if (!GhostPlayer) {
      continue;
    }
    GhostPlayer->SetUserId(Ghost.PlayerName.empty() ? Ghost.UserId : Ghost.PlayerName);
    GhostPlayer->SetGhostData(Frames);
    GhostPlayers.push_back(GhostPlayer);
  }
  M_LOG(
      Log, "Spawned {} local race ghosts for connection {}", GhostPlayers.size(), OwnerConnectionId
  );
}

void PC_Game::HandleNetworkPacket(FNetworkConnectionId ConnectionId, FNetBuffer& Buffer) {
  (void)ConnectionId;
  ENetPacketType PacketType = ENetPacketType::None;
  if (!Buffer.Read(PacketType) || PacketType != ENetPacketType::UserMessage) {
    return;
  }
  uint8_t MessageType = 0;
  if (!Buffer.Read(MessageType)) {
    return;
  }
  if (MessageType == static_cast<uint8_t>(ECatsRaceMessageType::RaceStartTime)) {
    double StartTime = 0.0;
    if (Buffer.Read(StartTime)) {
      ReceiveRaceStartTime(StartTime);
    }
  }
}

void PC_Game::ReceiveRaceStartTime(double StartTime) {
  if (RaceRunning || bHasRaceStartTime) {
    return;
  }
  RaceStartServerTime = StartTime;
  bHasRaceStartTime = true;
  M_LOG(
      Log,
      "Race start time received: connection={} local={} start_time={} estimated_server_time={}",
      OwnerConnectionId,
      bIsLocallyControlled,
      RaceStartServerTime,
      NetworkManager::GetInstance().GetEstimatedServerTime()
  );
}

void PC_Game::UpdateCountdown(double RemainingTime) {
  const int Count = static_cast<int>(std::ceil(RemainingTime));
  if (Count < 1 || Count > 3 || Count == LastDisplayedCount) {
    return;
  }
  LastDisplayedCount = Count;
  if (CountDownWidget) {
    CountDownWidget->SetCountText(std::to_string(Count));
  }
  GetWorld()->GetSoundManager()->PlaySE("/Game/soundreality-pop-423717.mp3", false);
}

void PC_Game::StartLocalRace() {
  if (RaceRunning) {
    return;
  }
  RaceRunning = true;
  RaceTime = 0.0f;
  if (CountDownWidget) {
    CountDownWidget->SetCountText("Go!");
  }
  SetInputMode(EInputMode::GameOnly);
  if (PlayerDirectionIndicator) {
    PlayerDirectionIndicator->InitializePlayers(dynamic_cast<APlayer*>(GetPawn()));
  }
  GetWorldTimerManager().SetTimer(CountHandle, this, &PC_Game::ClearCountDown, 1.0f, false, 1.0f);
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
    Mapper->AddMapping(InputActionLower::MoveY, kb, EKey::W, "", 1.0f);
    Mapper->AddMapping(InputActionLower::MoveY, kb, EKey::S, "", -1.0f);
    Mapper->AddMapping(InputAction::Interact, kb, EKey::F);
    Mapper->AddMapping(InputAction::Pause, kb, EKey::Escape);
    Mapper->AddMapping("DRIFT", kb, EKey::Space);
    Mapper->AddMapping("USE_ITEM", kb, EKey::E);
  }
  if (pad) {
    // ゲームプレイ中の MoveY には左スティックをバインドせず、トリガーのみで操作させる
    Mapper->AddAxisMapping(
        InputActionLower::MoveY, pad, static_cast<int>(AxisID::RightTrigger), 1.0f
    );
    Mapper->AddAxisMapping(
        InputActionLower::MoveY, pad, static_cast<int>(AxisID::LeftTrigger), -1.0f
    );

    Mapper->AddMapping(InputAction::Interact, pad, EGamepadButton::North);
    Mapper->AddMapping("DRIFT", pad, EGamepadButton::South);
    Mapper->AddMapping("USE_ITEM", pad, EGamepadButton::West);
  }
}
