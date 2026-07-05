#include "Scenes/Loading/LoadingScene.h"

#include <DxLib.h>
#include <cstring>
#include <memory>
#include <string>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "OnlineSessionManager.h"
#include "PlayerController.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "UITextComponent.h"
#include "World.h"

REGISTER_GAME_MODE(ALoadingScene)

ALoadingScene::ALoadingScene() {
  SetUpdateableAnytime(true);

  auto background = std::make_unique<MSpriteComponent>(-10, RenderSpace::Screen);
  background->SetRelativeLocation({0.0f, 0.0f});
  background->SubmitBox(1920.0f, 1080.0f, GetColor(0, 0, 0), true);
  AddComponent(std::move(background));

  auto titleText = std::make_unique<UITextComponent>("Connecting to Online Services...", 0xFFFFFF, 32);
  titleText->SetAnchor(EUIAnchor::MiddleCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, -36.0f});
  AddComponent(std::move(titleText));

  auto statusText = std::make_unique<UITextComponent>("Logging in...", 0xB8C7D9, 22);
  StatusText = statusText.get();
  StatusText->SetAnchor(EUIAnchor::MiddleCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, 24.0f});
  AddComponent(std::move(statusText));
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

  OnlineSessionManager& onlineSession = OnlineSessionManager::Get();
  if (!onlineSession.IsEOSInitialized()) {
    SetStatusMessage("Online services are not initialized.");
    return;
  }

  if (onlineSession.IsLoggedIn()) {
    SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu);
    return;
  }

  SetStatusMessage("Logging in...");
  if (!onlineSession.LoginWithDeviceId(PlayerName, [this](bool bSuccess) {
        if (bSuccess) {
          SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu);
        } else {
          SetStatusMessage("Login failed. Check logs and restart the game.");
        }
      })) {
    SetStatusMessage("Login request was rejected.");
  }
}

void ALoadingScene::SetStatusMessage(const std::string& Message) {
  if (StatusText) {
    StatusText->SetText(Message);
  }
}