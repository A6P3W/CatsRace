#include "WMainHUD.h"
#include <SpriteComponent.h>
#include <UITextComponent.h>
#include "ResourceManager.h"
#include <iomanip>
#include <sstream>
 
WMainHUD::WMainHUD() {
  auto text = std::make_unique<UITextComponent>("Time: 0.00", 0x00FF88, 24);
  m_TimerText = text.get();
  m_TimerText->SetAnchor(EUIAnchor::TopLeft);
  m_TimerText->SetAnchoredPosition({100.0f, 100.0f});
  AddComponent(std::move(text));

    auto itemIcon = std::make_unique<MSpriteComponent>(10, RenderSpace::Screen);
  m_ItemIcon = itemIcon.get();
  m_ItemIcon->SetRelativeLocation({80.0f, 900.0f});
  int handle = ResourceManager::GetInstance().LoadResourceGraph("Resources/images/speedup2.png");
  m_ItemIcon->SubmitGraph(handle, FScale(0.3f), 200);
  m_ItemIcon->SetVisibility(false);
  AddComponent(std::move(itemIcon));

  auto itemText = std::make_unique<UITextComponent>("キノコ", 0xFFFFFF, 22);
  m_ItemText = itemText.get();
  m_ItemText->SetAnchor(EUIAnchor::BottomLeft);
  m_ItemText->SetPivot({0.0f, 1.0f});
  m_ItemText->SetAnchoredPosition({55.0f, -20.0f});
  m_ItemText->SetVisibility(false);
  AddComponent(std::move(itemText));

    auto lapText = std::make_unique<UITextComponent>("LAP 1 / 3", 0xFFFFFF, 28);
  m_LapText = lapText.get();
  m_LapText->SetAnchor(EUIAnchor::TopRight);
  m_LapText->SetAnchoredPosition({-120.0f, 60.0f});
  AddComponent(std::move(lapText));
}

void WMainHUD::UpdateTimerText(float elapsedTime) {




  std::ostringstream oss;
  oss << "Time: " << std::fixed << std::setprecision(2) << elapsedTime;
  m_TimerText->SetText(oss.str());
}



void WMainHUD::SetHeldItemVisible(bool bVisible) {
  if (m_ItemIcon) m_ItemIcon->SetVisibility(bVisible);
  if (m_ItemText) m_ItemText->SetVisibility(bVisible);
}
void WMainHUD::UpdateLapText(int currentLap, int totalLaps) {
  if (m_LapText) {
    m_LapText->SetText("LAP " + std::to_string(currentLap + 1) + " / " + std::to_string(totalLaps));
  }
}
