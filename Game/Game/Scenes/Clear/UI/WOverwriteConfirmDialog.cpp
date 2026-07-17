#include "WOverwriteConfirmDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

WOverwriteConfirmDialog::WOverwriteConfirmDialog() {
  // Background Panel (ZOrder: 0)
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 150.0f});
  m_BgPanel->SubmitBox(600, 300, FColor{15, 15, 20, 220}, true);
  m_BgPanel->RegisterComponent();

  // Title Text (Warning style, ZOrder: 1)
  m_TitleText = NewObject<UITextComponent>(this);
  m_TitleText->SetText("この名前は登録済みです");
  m_TitleText->SetColor(FColor{255, 85, 85});
  m_TitleText->SetFontSize(26);
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -80.0f});
  m_TitleText->RegisterComponent();

  // 1. Overwrite Button (Left, ZOrder: 1)
  m_BtnOverwrite = NewObject<UIBoxButtonComponent>(this);
  m_BtnOverwrite->SetSize(200.0f, 45.0f);
  m_BtnOverwrite->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnOverwrite->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnOverwrite->SetPivot({0.5f, 0.5f});
  m_BtnOverwrite->SetAnchoredPosition({-120.0f, 50.0f});
  m_BtnOverwrite->RegisterComponent();

  m_TxtOverwrite = NewObject<UITextComponent>(this);
  m_TxtOverwrite->SetText("上書き");
  m_TxtOverwrite->SetColor(FColor{255, 255, 255});
  m_TxtOverwrite->SetFontSize(18);
  m_TxtOverwrite->AttachToComponent(m_BtnOverwrite);
  m_TxtOverwrite->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtOverwrite->SetPivot({0.5f, 0.5f});
  m_TxtOverwrite->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtOverwrite->RegisterComponent();

  // 2. Re-enter Button (Right, ZOrder: 1)
  m_BtnReEnter = NewObject<UIBoxButtonComponent>(this);
  m_BtnReEnter->SetSize(200.0f, 45.0f);
  m_BtnReEnter->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnReEnter->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnReEnter->SetPivot({0.5f, 0.5f});
  m_BtnReEnter->SetAnchoredPosition({120.0f, 50.0f});
  m_BtnReEnter->RegisterComponent();

  m_TxtReEnter = NewObject<UITextComponent>(this);
  m_TxtReEnter->SetText("再入力");
  m_TxtReEnter->SetColor(FColor{255, 255, 255});
  m_TxtReEnter->SetFontSize(18);
  m_TxtReEnter->AttachToComponent(m_BtnReEnter);
  m_TxtReEnter->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtReEnter->SetPivot({0.5f, 0.5f});
  m_TxtReEnter->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtReEnter->RegisterComponent();
}

void WOverwriteConfirmDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  // Navigation setup (horizontal)
  m_BtnOverwrite->Navigation.Right = m_BtnReEnter;
  m_BtnReEnter->Navigation.Left = m_BtnOverwrite;

  // Initial focus setup to prevent accidental overwrites
  SetFocusedButton(m_BtnReEnter);

  // Button callback setup
  m_BtnOverwrite->SetOnPressed([this]() {
    if (m_Callback) {
      m_Callback(EOverwriteResult::Overwrite);
    }
  });

  m_BtnReEnter->SetOnPressed([this]() {
    if (m_Callback) {
      m_Callback(EOverwriteResult::ReEnter);
    }
  });
}

void WOverwriteConfirmDialog::SetOnResult(FOnOverwriteResult callback) { m_Callback = callback; }
