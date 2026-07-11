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
  auto* bgWidgetPtr = NewObject<MSpriteComponent>(this);
  bgWidgetPtr->SubmitBox(1920.0f, 1080.0f, GetColor(20, 20, 20), true, 220);
  bgWidgetPtr->RegisterComponent();

  auto* bgSprite = NewObject<MSpriteComponent>(this);
  bgSprite->SetRenderSettings(-10, RenderSpace::Screen);
  bgSprite->AttachToComponent(bgWidgetPtr);
  bgSprite->SetRelativeLocation({-1920.0f * 0.5f, -1080.0f * 0.5f});
  bgSprite->SubmitBox(1920.0f, 1080.0f, GetColor(0, 0, 0), true, 180);
  bgSprite->RegisterComponent();

  // 3. タイトルテキスト "PAUSE"
  m_TxtTitle = NewObject<UITextComponent>(this);
  m_TxtTitle->SetText("PAUSE");
  m_TxtTitle->SetColor(0xFFFFFF);
  m_TxtTitle->SetFontSize(64);
  m_TxtTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtTitle->SetPivot({0.5f, 0.5f});
  m_TxtTitle->SetAnchoredPosition({0.0f, -250.0f});
  m_TxtTitle->RegisterComponent();

  m_ButtonList = NewObject<MUIVerticalBoxComponent>(this);
  m_ButtonList->SetAnchor(EUIAnchor::MiddleCenter);
  m_ButtonList->SetPivot({0.5f, 0.5f});
  m_ButtonList->SetWidgetSize({PauseButtonWidth, 1.0f});
  m_ButtonList->SetAnchoredPosition({0.0f, -50.0f});
  m_ButtonList->SetSpacing(30.0f);
  m_ButtonList->RegisterComponent();
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
  auto* buttonPtr = NewObject<UIBoxButtonComponent>(this);
  buttonPtr->SetSize(PauseButtonWidth, PauseButtonHeight);
  buttonPtr->SetColors(ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor);
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (m_ButtonList) {
    m_ButtonList->AddItem(buttonPtr);
  }
  buttonPtr->RegisterComponent();

  auto* text = NewObject<UITextComponent>(this);
  text->SetText(Label);
  text->SetColor(0xFFFFFF);
  text->SetFontSize(24);
  text->AttachToComponent(buttonPtr);
  text->SetAnchor(EUIAnchor::MiddleCenter);
  text->SetPivot({0.5f, 0.5f});
  text->SetAnchoredPosition({0.0f, 0.0f});
  text->RegisterComponent();

  return buttonPtr;
}
