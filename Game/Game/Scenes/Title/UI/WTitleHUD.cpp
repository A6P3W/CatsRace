#include "WTitleHUD.h"

#include <Application.h>
#include <DxLib.h>
#include <SceneManager.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

#include "Core/GameSceneIds.h"

WTitleHUD::WTitleHUD() {
  // 1. Game Start Button (Left)
  auto btnStart = std::make_unique<UIBoxButtonComponent>(
      300.0f, 70.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnStart = btnStart.get();
  m_BtnStart->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnStart->SetPivot({0.5f, 0.5f});
  m_BtnStart->SetAnchoredPosition({-400.0f, 350.0f});
  m_BtnStart->SetParentComponent(nullptr);

  auto txtStart = std::make_unique<UITextComponent>("Start", 0xFFFFFF, 24);
  m_TxtStart = txtStart.get();
  m_TxtStart->SetParentComponent(m_BtnStart);
  m_TxtStart->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtStart->SetPivot({0.5f, 0.5f});
  m_TxtStart->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(btnStart));
  AddComponent(std::move(txtStart));

  // 2. Quit Game Button (Right)
  auto btnQuit = std::make_unique<UIBoxButtonComponent>(
      300.0f, 70.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnQuit = btnQuit.get();
  m_BtnQuit->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnQuit->SetPivot({0.5f, 0.5f});
  m_BtnQuit->SetAnchoredPosition({400.0f, 350.0f});
  m_BtnQuit->SetParentComponent(nullptr);

  auto txtQuit = std::make_unique<UITextComponent>("Quit", 0xFFFFFF, 24);
  m_TxtQuit = txtQuit.get();
  m_TxtQuit->SetParentComponent(m_BtnQuit);
  m_TxtQuit->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtQuit->SetPivot({0.5f, 0.5f});
  m_TxtQuit->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(btnQuit));
  AddComponent(std::move(txtQuit));
}

void WTitleHUD::BeginPlay() {
  AWidgetBase::BeginPlay();

  // Navigation setup
  m_BtnStart->Navigation.Right = m_BtnQuit;
  m_BtnQuit->Navigation.Left = m_BtnStart;

  // Initial focus setup
  SetFocusedButton(m_BtnStart);

  // Button callbacks
  m_BtnStart->OnPressed = [this]() {
    SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu);
  };

  m_BtnQuit->OnPressed = [this]() { Application::QuitGame(); };
}
