#include "Scenes/Lobby/UI/WLeaveLobbyConfirmDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

namespace {
constexpr float PanelWidth = 560.0f;
constexpr float PanelHeight = 260.0f;
constexpr float ButtonWidth = 180.0f;
constexpr float ButtonHeight = 48.0f;
}

WLeaveLobbyConfirmDialog::WLeaveLobbyConfirmDialog() {
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f - PanelWidth * 0.5f, 540.0f - PanelHeight * 0.5f});
  m_BgPanel->SubmitBox(static_cast<int>(PanelWidth), static_cast<int>(PanelHeight), FColor{15, 15, 20, 220}, true);
  m_BgPanel->RegisterComponent();

  m_MessageText = NewObject<UITextComponent>(this);
  m_MessageText->SetText("ロビーを退出しますか？");
  m_MessageText->SetColor(FColor::White);
  m_MessageText->SetFontSize(28);
  m_MessageText->SetAnchor(EUIAnchor::MiddleCenter);
  m_MessageText->SetPivot({0.5f, 0.5f});
  m_MessageText->SetAnchoredPosition({0.0f, -50.0f});
  m_MessageText->RegisterComponent();

  m_YesButton = NewObject<UIBoxButtonComponent>(this);
  m_YesButton->SetSize(ButtonWidth, ButtonHeight);
  m_YesButton->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_YesButton->SetAnchor(EUIAnchor::MiddleCenter);
  m_YesButton->SetPivot({0.5f, 0.5f});
  m_YesButton->SetAnchoredPosition({-110.0f, 55.0f});
  m_YesButton->RegisterComponent();

  auto* yesText = NewObject<UITextComponent>(this);
  yesText->SetText("YES");
  yesText->SetColor(FColor::White);
  yesText->SetFontSize(22);
  yesText->AttachToComponent(m_YesButton);
  yesText->SetAnchor(EUIAnchor::MiddleCenter);
  yesText->SetPivot({0.5f, 0.5f});
  yesText->SetAnchoredPosition({0.0f, 0.0f});
  yesText->RegisterComponent();

  m_NoButton = NewObject<UIBoxButtonComponent>(this);
  m_NoButton->SetSize(ButtonWidth, ButtonHeight);
  m_NoButton->SetColors(FColor{60, 50, 50}, FColor{150, 70, 70}, FColor{110, 45, 45});
  m_NoButton->SetAnchor(EUIAnchor::MiddleCenter);
  m_NoButton->SetPivot({0.5f, 0.5f});
  m_NoButton->SetAnchoredPosition({110.0f, 55.0f});
  m_NoButton->RegisterComponent();

  auto* noText = NewObject<UITextComponent>(this);
  noText->SetText("NO");
  noText->SetColor(FColor::White);
  noText->SetFontSize(22);
  noText->AttachToComponent(m_NoButton);
  noText->SetAnchor(EUIAnchor::MiddleCenter);
  noText->SetPivot({0.5f, 0.5f});
  noText->SetAnchoredPosition({0.0f, 0.0f});
  noText->RegisterComponent();
}

void WLeaveLobbyConfirmDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  m_YesButton->Navigation.Right = m_NoButton;
  m_NoButton->Navigation.Left = m_YesButton;
  SetFocusedButton(m_NoButton);

  m_YesButton->SetOnPressed([this]() { NotifyResult(true); });
  m_NoButton->SetOnPressed([this]() { NotifyResult(false); });
}

void WLeaveLobbyConfirmDialog::Cancel() { NotifyResult(false); }

void WLeaveLobbyConfirmDialog::NotifyResult(bool bLeaveLobby) {
  if (OnResult) {
    OnResult(bLeaveLobby);
  }
}
