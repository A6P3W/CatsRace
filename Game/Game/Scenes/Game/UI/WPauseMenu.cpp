#include "WPauseMenu.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>
#include <UIVerticalBoxComponent.h>
#include <World.h>

namespace {
constexpr float PauseButtonWidth = 350.0f;
constexpr float PauseButtonHeight = 70.0f;
constexpr int ButtonNormalColor = 0x282D37;
constexpr int ButtonHoveredColor = 0x0078D7;
constexpr int ButtonPressedColor = 0x005AA0;
}  // namespace

WPauseMenu::WPauseMenu() {
  // 1. 一時停止中でも更新を許可する
  SetUpdateableAnytime(true);

  // 2. 画面全体を覆う半透明背景
  auto bgWidget = std::make_unique<MSpriteComponent>();
  bgWidget->SubmitBox(1920.0f, 1080.0f, GetColor(20, 20, 20), true, 220);
  auto* bgWidgetPtr = bgWidget.get();
  AddComponent(std::move(bgWidget));

  auto bgSprite = std::make_unique<MSpriteComponent>(-10, RenderSpace::Screen);
  bgSprite->SetParentComponent(bgWidgetPtr);
  bgSprite->SetRelativeLocation({-1920.0f * 0.5f, -1080.0f * 0.5f});
  bgSprite->SubmitBox(1920.0f, 1080.0f, GetColor(0, 0, 0), true, 180);
  AddComponent(std::move(bgSprite));

  // 3. タイトルテキスト "PAUSE"
  auto txtTitle = std::make_unique<UITextComponent>("PAUSE", 0xFFFFFF, 64);
  m_TxtTitle = txtTitle.get();
  m_TxtTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtTitle->SetPivot({0.5f, 0.5f});
  m_TxtTitle->SetAnchoredPosition({0.0f, -250.0f});
  AddComponent(std::move(txtTitle));

  auto buttonList = std::make_unique<MUIVerticalBoxComponent>();
  m_ButtonList = buttonList.get();
  m_ButtonList->SetAnchor(EUIAnchor::MiddleCenter);
  m_ButtonList->SetPivot({0.5f, 0.5f});
  m_ButtonList->SetWidgetSize({PauseButtonWidth, 1.0f});
  m_ButtonList->SetAnchoredPosition({0.0f, -50.0f});
  m_ButtonList->SetSpacing(30.0f);
  AddComponent(std::move(buttonList));
}

void WPauseMenu::BeginPlay() {
  AWidgetBase::BeginPlay();

  m_BtnResume = AddMenuButton("再開");
  m_BtnResume->OnPressed = [this]() {
    if (OnResumePressed) OnResumePressed();
  };

  if (GetWorld() && GetWorld()->IsServer()) {
    m_BtnRestart = AddMenuButton("最初から");
    m_BtnRestart->OnPressed = [this]() {
      if (OnRestartPressed) OnRestartPressed();
    };

    m_BtnTitle = AddMenuButton("ロビーに戻る");
    m_BtnTitle->OnPressed = [this]() {
      if (OnTitlePressed) OnTitlePressed();
    };
  } else if (GetWorld() && GetWorld()->IsClient()) {
    m_BtnLeave = AddMenuButton("退出");
    m_BtnLeave->OnPressed = [this]() {
      if (OnLeavePressed) OnLeavePressed();
    };
  }

  if (m_ButtonList) {
    m_ButtonList->BuildNavigation();
  }
  SetFocusedButton(m_BtnResume);
}

void WPauseMenu::Cancel() {
  // キャンセル（ESCキーなど）で再開する
  if (OnResumePressed) OnResumePressed();
}

UIBoxButtonComponent* WPauseMenu::AddMenuButton(const std::string& Label) {
  auto button = std::make_unique<UIBoxButtonComponent>(
      PauseButtonWidth, PauseButtonHeight, ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor
  );
  UIBoxButtonComponent* buttonPtr = button.get();
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (m_ButtonList) {
    m_ButtonList->AddItem(buttonPtr);
  }

  auto text = std::make_unique<UITextComponent>(Label, 0xFFFFFF, 24);
  text->SetParentComponent(buttonPtr);
  text->SetAnchor(EUIAnchor::MiddleCenter);
  text->SetPivot({0.5f, 0.5f});
  text->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(button));
  AddComponent(std::move(text));
  return buttonPtr;
}
