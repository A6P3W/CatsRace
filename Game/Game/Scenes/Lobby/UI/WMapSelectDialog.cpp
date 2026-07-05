#include "Scenes/Lobby/UI/WMapSelectDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

#include <algorithm>
#include <memory>

#include "Core/MapData.h"

WMapSelectDialog::WMapSelectDialog() {
  auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
  m_BgPanel = bgPanel.get();
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 210.0f});
  m_BgPanel->SubmitBox(600, 420, GetColor(15, 15, 20), true, 220);
  m_BgPanel->SetParentComponent(nullptr);
  AddComponent(std::move(bgPanel));

  auto titleText = std::make_unique<UITextComponent>("Select Map", 0xFFFFFF, 30);
  m_TitleText = titleText.get();
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -150.0f});
  m_TitleText->SetParentComponent(nullptr);
  AddComponent(std::move(titleText));

  constexpr float ButtonWidth = 400.0f;
  constexpr float ButtonHeight = 46.0f;
  constexpr float ButtonSpacing = 14.0f;
  const float itemPitch = ButtonHeight + ButtonSpacing;
  const float firstY = -70.0f - itemPitch * (static_cast<float>(AvailableMaps.size()) - 1.0f) * 0.5f;

  for (size_t index = 0; index < AvailableMaps.size(); ++index) {
    const auto& mapInfo = AvailableMaps[index];
    auto button = std::make_unique<UIBoxButtonComponent>(
        ButtonWidth, ButtonHeight, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
    );
    auto* buttonPtr = button.get();
    buttonPtr->SetAnchor(EUIAnchor::MiddleCenter);
    buttonPtr->SetPivot({0.5f, 0.5f});
    buttonPtr->SetAnchoredPosition({0.0f, firstY + itemPitch * static_cast<float>(index)});
    buttonPtr->SetParentComponent(nullptr);
    m_MapButtons.push_back(buttonPtr);

    auto text = std::make_unique<UITextComponent>(mapInfo.DisplayName, 0xFFFFFF, 22);
    text->SetParentComponent(buttonPtr);
    text->SetAnchor(EUIAnchor::MiddleCenter);
    text->SetPivot({0.5f, 0.5f});
    text->SetAnchoredPosition({0.0f, 0.0f});

    AddComponent(std::move(text));
    AddComponent(std::move(button));
  }

  auto btnCancel = std::make_unique<UIBoxButtonComponent>(
      180.0f, 42.0f, GetColor(60, 50, 50), GetColor(150, 70, 70), GetColor(110, 45, 45)
  );
  m_BtnCancel = btnCancel.get();
  m_BtnCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnCancel->SetPivot({0.5f, 0.5f});
  m_BtnCancel->SetAnchoredPosition({0.0f, 130.0f});
  m_BtnCancel->SetParentComponent(nullptr);

  auto txtCancel = std::make_unique<UITextComponent>("Cancel", 0xFFFFFF, 20);
  m_TxtCancel = txtCancel.get();
  m_TxtCancel->SetParentComponent(m_BtnCancel);
  m_TxtCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtCancel->SetPivot({0.5f, 0.5f});
  m_TxtCancel->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(txtCancel));
  AddComponent(std::move(btnCancel));
}

void WMapSelectDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  const size_t mapButtonCount = m_MapButtons.size() < AvailableMaps.size() ? m_MapButtons.size() : AvailableMaps.size();
  for (size_t index = 0; index < mapButtonCount; ++index) {
    auto* button = m_MapButtons[index];
    if (!button) {
      continue;
    }

    const std::string levelPath = AvailableMaps[index].LevelPath;
    button->OnPressed = [this, levelPath]() { NotifySelected(levelPath); };

    button->Navigation.Up = index == 0 ? m_BtnCancel : m_MapButtons[index - 1];
    button->Navigation.Down = index + 1 < mapButtonCount ? m_MapButtons[index + 1] : m_BtnCancel;
  }

  if (m_BtnCancel) {
    m_BtnCancel->OnPressed = [this]() { NotifySelected(""); };
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

