#include "Scenes/Lobby/UI/WMapSelectDialog.h"

#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

#include <algorithm>
#include <memory>

#include "Core/MapData.h"

WMapSelectDialog::WMapSelectDialog() {
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f, 540.0f});
  m_BgPanel->SubmitBox(600, 420, FColor{15, 15, 20, 220}, true);
  m_BgPanel->RegisterComponent();

  m_TitleText = NewObject<UITextComponent>(this);
  m_TitleText->SetText("Select Map");
  m_TitleText->SetColor(FColor{255, 255, 255});
  m_TitleText->SetFontSize(30);
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -150.0f});
  m_TitleText->RegisterComponent();

  constexpr float ButtonWidth = 400.0f;
  constexpr float ButtonHeight = 46.0f;
  constexpr float ButtonSpacing = 14.0f;
  const float itemPitch = ButtonHeight + ButtonSpacing;
  const float firstY =
      -70.0f - itemPitch * (static_cast<float>(AvailableMaps.size()) - 1.0f) * 0.5f;

  for (size_t index = 0; index < AvailableMaps.size(); ++index) {
    const auto& mapInfo = AvailableMaps[index];
    auto* buttonPtr = NewObject<UIBoxButtonComponent>(this);
    buttonPtr->SetSize(ButtonWidth, ButtonHeight);
    buttonPtr->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
    buttonPtr->SetAnchor(EUIAnchor::MiddleCenter);
    buttonPtr->SetPivot({0.5f, 0.5f});
    buttonPtr->SetAnchoredPosition({0.0f, firstY + itemPitch * static_cast<float>(index)});
    m_MapButtons.push_back(buttonPtr);
    buttonPtr->RegisterComponent();

    auto* text = NewObject<UITextComponent>(this);
    text->SetText(mapInfo.DisplayName);
    text->SetColor(FColor{255, 255, 255});
    text->SetFontSize(22);
    text->AttachToComponent(buttonPtr);
    text->SetAnchor(EUIAnchor::MiddleCenter);
    text->SetPivot({0.5f, 0.5f});
    text->SetAnchoredPosition({0.0f, 0.0f});
    text->RegisterComponent();
  }

  m_BtnCancel = NewObject<UIBoxButtonComponent>(this);
  m_BtnCancel->SetSize(180.0f, 42.0f);
  m_BtnCancel->SetColors(FColor{60, 50, 50}, FColor{150, 70, 70}, FColor{110, 45, 45});
  m_BtnCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnCancel->SetPivot({0.5f, 0.5f});
  m_BtnCancel->SetAnchoredPosition({0.0f, 130.0f});
  m_BtnCancel->RegisterComponent();

  m_TxtCancel = NewObject<UITextComponent>(this);
  m_TxtCancel->SetText("Cancel");
  m_TxtCancel->SetColor(FColor{255, 255, 255});
  m_TxtCancel->SetFontSize(20);
  m_TxtCancel->AttachToComponent(m_BtnCancel);
  m_TxtCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtCancel->SetPivot({0.5f, 0.5f});
  m_TxtCancel->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtCancel->RegisterComponent();
}

void WMapSelectDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  const size_t mapButtonCount =
      m_MapButtons.size() < AvailableMaps.size() ? m_MapButtons.size() : AvailableMaps.size();
  for (size_t index = 0; index < mapButtonCount; ++index) {
    auto* button = m_MapButtons[index];
    if (!button) {
      continue;
    }

    const std::string levelPath = AvailableMaps[index].LevelPath;
    button->SetOnPressed([this, levelPath]() { NotifySelected(levelPath); });

    button->Navigation.Up = index == 0 ? m_BtnCancel : m_MapButtons[index - 1];
    button->Navigation.Down = index + 1 < mapButtonCount ? m_MapButtons[index + 1] : m_BtnCancel;
  }

  if (m_BtnCancel) {
    m_BtnCancel->SetOnPressed([this]() { NotifySelected(""); });
    if (mapButtonCount > 0) {
      m_BtnCancel->Navigation.Up = m_MapButtons[mapButtonCount - 1];
      m_BtnCancel->Navigation.Down = m_MapButtons.front();
    }
  }

  SetFocusedButton(mapButtonCount > 0 ? m_MapButtons.front() : m_BtnCancel);
}

void WMapSelectDialog::Cancel() { NotifySelected(""); }

void WMapSelectDialog::NotifySelected(const std::string& LevelPath) {
  if (OnMapSelected) {
    OnMapSelected(LevelPath);
  }
}
