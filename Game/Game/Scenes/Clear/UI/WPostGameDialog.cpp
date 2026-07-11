#include "WPostGameDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

WPostGameDialog::WPostGameDialog() {
  // Background Panel (ZOrder: 0)
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 150.0f});
  m_BgPanel->SubmitBox(600, 300, GetColor(15, 15, 20), true, 220);
  m_BgPanel->RegisterComponent();

  // 1. Play Again Button (Left, ZOrder: 1)
  m_BtnPlayAgain = NewObject<UIBoxButtonComponent>(this);
  m_BtnPlayAgain->SetSize(200.0f, 45.0f);
  m_BtnPlayAgain->SetColors(GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
  m_BtnPlayAgain->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnPlayAgain->SetPivot({0.5f, 0.5f});
  m_BtnPlayAgain->SetAnchoredPosition({-120.0f, 50.0f});
  m_BtnPlayAgain->RegisterComponent();

  m_TxtPlayAgain = NewObject<UITextComponent>(this);
  m_TxtPlayAgain->SetText("もう一度プレイ");
  m_TxtPlayAgain->SetColor(0xFFFFFF);
  m_TxtPlayAgain->SetFontSize(18);
  m_TxtPlayAgain->AttachToComponent(m_BtnPlayAgain);
  m_TxtPlayAgain->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtPlayAgain->SetPivot({0.5f, 0.5f});
  m_TxtPlayAgain->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtPlayAgain->RegisterComponent();

  // 2. Back To Title Button (Right, ZOrder: 1)
  m_BtnBackToTitle = NewObject<UIBoxButtonComponent>(this);
  m_BtnBackToTitle->SetSize(200.0f, 45.0f);
  m_BtnBackToTitle->SetColors(GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
  m_BtnBackToTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnBackToTitle->SetPivot({0.5f, 0.5f});
  m_BtnBackToTitle->SetAnchoredPosition({120.0f, 50.0f});
  m_BtnBackToTitle->RegisterComponent();

  m_TxtBackToTitle = NewObject<UITextComponent>(this);
  m_TxtBackToTitle->SetText("タイトルへ戻る");
  m_TxtBackToTitle->SetColor(0xFFFFFF);
  m_TxtBackToTitle->SetFontSize(18);
  m_TxtBackToTitle->AttachToComponent(m_BtnBackToTitle);
  m_TxtBackToTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtBackToTitle->SetPivot({0.5f, 0.5f});
  m_TxtBackToTitle->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtBackToTitle->RegisterComponent();
}

void WPostGameDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  // Navigation setup (horizontal)
  m_BtnPlayAgain->Navigation.Right = m_BtnBackToTitle;
  m_BtnBackToTitle->Navigation.Left = m_BtnPlayAgain;

  // Initial focus setup to Play Again
  SetFocusedButton(m_BtnPlayAgain);

  // Button callback setup
  m_BtnPlayAgain->OnPressed = [this]() {
    if (m_Callback) {
      m_Callback(EPostGameResult::PlayAgain);
    }
  };

  m_BtnBackToTitle->OnPressed = [this]() {
    if (m_Callback) {
      m_Callback(EPostGameResult::BackToTitle);
    }
  };
}

void WPostGameDialog::SetOnResult(FOnPostGameResult callback) { m_Callback = callback; }
