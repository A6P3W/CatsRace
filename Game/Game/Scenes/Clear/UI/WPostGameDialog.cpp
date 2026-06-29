#include "WPostGameDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

WPostGameDialog::WPostGameDialog() {
  // Background Panel (ZOrder: 0)
  auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
  m_BgPanel = bgPanel.get();
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 150.0f});
  m_BgPanel->SubmitBox(600, 300, GetColor(15, 15, 20), true, 220);
  m_BgPanel->SetParentComponent(nullptr);
  AddComponent(std::move(bgPanel));

  // 1. Play Again Button (Left, ZOrder: 1)
  auto btnPlayAgain = std::make_unique<UIBoxButtonComponent>(
      200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnPlayAgain = btnPlayAgain.get();
  m_BtnPlayAgain->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnPlayAgain->SetPivot({0.5f, 0.5f});
  m_BtnPlayAgain->SetAnchoredPosition({-120.0f, 50.0f});
  m_BtnPlayAgain->SetParentComponent(nullptr);

  auto txtPlayAgain = std::make_unique<UITextComponent>("もう一度プレイ", 0xFFFFFF, 18);
  m_TxtPlayAgain = txtPlayAgain.get();
  m_TxtPlayAgain->SetParentComponent(m_BtnPlayAgain);
  m_TxtPlayAgain->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtPlayAgain->SetPivot({0.5f, 0.5f});
  m_TxtPlayAgain->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(txtPlayAgain));
  AddComponent(std::move(btnPlayAgain));

  // 2. Back To Title Button (Right, ZOrder: 1)
  auto btnBackToTitle = std::make_unique<UIBoxButtonComponent>(
      200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnBackToTitle = btnBackToTitle.get();
  m_BtnBackToTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnBackToTitle->SetPivot({0.5f, 0.5f});
  m_BtnBackToTitle->SetAnchoredPosition({120.0f, 50.0f});
  m_BtnBackToTitle->SetParentComponent(nullptr);

  auto txtBackToTitle = std::make_unique<UITextComponent>("タイトルへ戻る", 0xFFFFFF, 18);
  m_TxtBackToTitle = txtBackToTitle.get();
  m_TxtBackToTitle->SetParentComponent(m_BtnBackToTitle);
  m_TxtBackToTitle->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtBackToTitle->SetPivot({0.5f, 0.5f});
  m_TxtBackToTitle->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(txtBackToTitle));
  AddComponent(std::move(btnBackToTitle));
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
