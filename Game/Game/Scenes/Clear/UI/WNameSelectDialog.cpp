#include "WNameSelectDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

WNameSelectDialog::WNameSelectDialog() {
  // Background Panel (ZOrder: 0)
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 175.0f});
  m_BgPanel->SubmitBox(600, 350, FColor{15, 15, 20, 220}, true);
  m_BgPanel->RegisterComponent();

  // Title Text (ZOrder: 1)
  m_TitleText = NewObject<UITextComponent>(this);
  m_TitleText->SetText("ランキングへの登録方法");
  m_TitleText->SetColor(FColor{255, 255, 255});
  m_TitleText->SetFontSize(28);
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -100.0f});
  m_TitleText->RegisterComponent();

  // 1. Use Existing Name Button
  m_BtnUseExisting = NewObject<UIBoxButtonComponent>(this);
  m_BtnUseExisting->SetSize(400.0f, 45.0f);
  m_BtnUseExisting->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnUseExisting->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnUseExisting->SetPivot({0.5f, 0.5f});
  m_BtnUseExisting->SetAnchoredPosition({0.0f, -20.0f});
  m_BtnUseExisting->RegisterComponent();

  m_TxtUseExisting = NewObject<UITextComponent>(this);
  m_TxtUseExisting->SetText("前回の名前を使用");
  m_TxtUseExisting->SetColor(FColor{255, 255, 255});
  m_TxtUseExisting->SetFontSize(20);
  m_TxtUseExisting->AttachToComponent(m_BtnUseExisting);
  m_TxtUseExisting->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtUseExisting->SetPivot({0.5f, 0.5f});
  m_TxtUseExisting->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtUseExisting->RegisterComponent();

  // 2. Enter New Name Button
  m_BtnEnterNew = NewObject<UIBoxButtonComponent>(this);
  m_BtnEnterNew->SetSize(400.0f, 45.0f);
  m_BtnEnterNew->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnEnterNew->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnEnterNew->SetPivot({0.5f, 0.5f});
  m_BtnEnterNew->SetAnchoredPosition({0.0f, 40.0f});
  m_BtnEnterNew->RegisterComponent();

  m_TxtEnterNew = NewObject<UITextComponent>(this);
  m_TxtEnterNew->SetText("新しい名前で登録");
  m_TxtEnterNew->SetColor(FColor{255, 255, 255});
  m_TxtEnterNew->SetFontSize(20);
  m_TxtEnterNew->AttachToComponent(m_BtnEnterNew);
  m_TxtEnterNew->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtEnterNew->SetPivot({0.5f, 0.5f});
  m_TxtEnterNew->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtEnterNew->RegisterComponent();

  // 3. Skip Button
  m_BtnSkip = NewObject<UIBoxButtonComponent>(this);
  m_BtnSkip->SetSize(400.0f, 45.0f);
  m_BtnSkip->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnSkip->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnSkip->SetPivot({0.5f, 0.5f});
  m_BtnSkip->SetAnchoredPosition({0.0f, 100.0f});
  m_BtnSkip->RegisterComponent();

  m_TxtSkip = NewObject<UITextComponent>(this);
  m_TxtSkip->SetText("登録しない");
  m_TxtSkip->SetColor(FColor{170, 170, 170});
  m_TxtSkip->SetFontSize(20);
  m_TxtSkip->AttachToComponent(m_BtnSkip);
  m_TxtSkip->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtSkip->SetPivot({0.5f, 0.5f});
  m_TxtSkip->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtSkip->RegisterComponent();
}

void WNameSelectDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  // Navigation link setup (vertical loop)
  m_BtnUseExisting->Navigation.Down = m_BtnEnterNew;
  m_BtnUseExisting->Navigation.Up = m_BtnSkip;

  m_BtnEnterNew->Navigation.Down = m_BtnSkip;
  m_BtnEnterNew->Navigation.Up = m_BtnUseExisting;

  m_BtnSkip->Navigation.Down = m_BtnUseExisting;
  m_BtnSkip->Navigation.Up = m_BtnEnterNew;

  // Initial focus setup
  SetFocusedButton(m_BtnUseExisting);

  // Button callback setup
  m_BtnUseExisting->SetOnPressed([this]() {
    if (m_Callback) {
      m_Callback(ENameSelectResult::UseExisting);
    }
  });

  m_BtnEnterNew->SetOnPressed([this]() {
    if (m_Callback) {
      m_Callback(ENameSelectResult::EnterNew);
    }
  });

  m_BtnSkip->SetOnPressed([this]() {
    if (m_Callback) {
      m_Callback(ENameSelectResult::Skip);
    }
  });
}

void WNameSelectDialog::SetExistingName(const std::string& name) {
  m_ExistingName = name;
  if (m_TxtUseExisting) {
    m_TxtUseExisting->SetText("登録名：" + name);
  }
}

void WNameSelectDialog::SetOnResult(FOnNameSelectResult callback) { m_Callback = callback; }
