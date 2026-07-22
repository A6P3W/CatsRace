#include "Scenes/Loading/LoadingScene.h"

#include <DxLib.h>

#include <array>
#include <cstring>
#include <memory>
#include <string>

#include "Application.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "EOSTitleStorageManager.h"
#include "OnlinePlayManager.h"
#include "PlayerController.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "UITextComponent.h"
#include "World.h"

namespace {
struct FLevelRegistration {
  const char* LocalPath;
  FNetworkSceneId SceneId;
};

constexpr const char* LevelsArchiveFileName = "Levels.zip";

constexpr std::array<FLevelRegistration, 4> LevelRegistrations = {{
    {"Resources-EOS/Stage1/Stage1.BLevel", GameSceneIds::Game01},
    {"Resources-EOS/Stage2/Stage2.BLevel", GameSceneIds::Game02},
    {"Resources-EOS/Stage3/Stage3.BLevel", GameSceneIds::Game03},
    {"Resources-EOS/Lobby/LobbyScene.BLevel", GameSceneIds::Lobby},
}};

constexpr float QuitDelaySeconds = 5.0f;
}  // namespace

REGISTER_GAME_MODE(ALoadingScene)

ALoadingScene::ALoadingScene() {
  SetUpdateableAnytime(true);

  auto* background = NewObject<MSpriteComponent>(this);
  background->SetRenderSettings(-10, RenderSpace::Screen);
  background->SetRelativeLocation({0.0f, 0.0f});
  background->SubmitBox(1920.0f, 1080.0f, FColor{0, 0, 0}, true);
  background->RegisterComponent();

  auto* titleText = NewObject<UITextComponent>(this);
  titleText->SetText("Connecting to Online Services...");
  titleText->SetColor(FColor{255, 255, 255});
  titleText->SetFontSize(32);
  titleText->SetAnchor(EUIAnchor::MiddleCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, -36.0f});
  titleText->RegisterComponent();

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("Logging in...");
  StatusText->SetColor(FColor{184, 199, 217});
  StatusText->SetFontSize(22);
  StatusText->SetAnchor(EUIAnchor::MiddleCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, 24.0f});
  StatusText->RegisterComponent();
}

void ALoadingScene::BeginPlay() {
  AGameModeBase::BeginPlay();

  if (auto* controller = GetWorld()->GetOrCreateLocalPlayerController()) {
    controller->SetInputMode(EInputMode::UIOnly);
  }

  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    const std::string name = gi->player_name.empty() ? gi->user_id : gi->player_name;
    if (!name.empty()) {
      strncpy_s(PlayerName, sizeof(PlayerName), name.c_str(), _TRUNCATE);
    }
  }

  OnlinePlayManager& onlinePlay = OnlinePlayManager::GetInstance();
  if (!onlinePlay.IsEOSInitialized()) {
    FailAndQuit("ログイン失敗。5秒後にゲームを終了します。");
    return;
  }

  if (onlinePlay.IsLoggedIn()) {
    StartLevelDownload();
    return;
  }

  SetStatusMessage("ログイン中...");
  onlinePlay.Login(PlayerName, [this](const FOnlinePlayResult& Result) {
    if (Result.Success) {
      StartLevelDownload();
    } else {
      FailAndQuit("ログイン失敗。5秒後にゲームを終了します。");
    }
  });
}

void ALoadingScene::OnUpdate(float DeltaTime) {
  AGameModeBase::OnUpdate(DeltaTime);
  if (!bFailed) {
    return;
  }

  QuitCountdown -= DeltaTime;
  if (QuitCountdown <= 0.0f) {
    bFailed = false;
    Application::QuitGame();
  }
}

void ALoadingScene::StartLevelDownload() {
  SetStatusMessage("ゲームレベルをロード中...");

  EOSTitleStorageManager::GetInstance().Download(
      LevelsArchiveFileName, [this](const FTitleStorageDownloadResult& Result) {
        if (bFailed) {
          return;
        }
        if (!Result.Success) {
          FailAndQuit("レベルロード失敗。5秒後にゲームを終了します。");
          return;
        }

        SceneManager& SceneManagerInstance = SceneManager::GetInstance();
        for (const FLevelRegistration& Level : LevelRegistrations) {
          SceneManagerInstance.RegisterLevelPath(Level.SceneId, Level.LocalPath);
        }
        SetStatusMessage("ゲームレベルのロードが完了しました。");
        SceneManagerInstance.OpenLevelById(GameSceneIds::Menu);
      }
  );
}

void ALoadingScene::FailAndQuit(const std::string& Message) {
  if (bFailed) {
    return;
  }
  bFailed = true;
  QuitCountdown = QuitDelaySeconds;
  SetStatusMessage(Message);
}

void ALoadingScene::SetStatusMessage(const std::string& Message) {
  if (StatusText) {
    StatusText->SetText(Message);
  }
}
