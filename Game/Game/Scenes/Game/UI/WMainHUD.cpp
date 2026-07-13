#include "WMainHUD.h"
#include <SpriteComponent.h>
#include <UITextComponent.h>
#include "ResourceManager.h"
#include <iomanip>
#include <sstream>
 
WMainHUD::WMainHUD() {
  m_TimerText = NewObject<UITextComponent>(this);
  m_TimerText->SetText("Time: 0.00");
  m_TimerText->SetColor(0x00FF88);
  m_TimerText->SetFontSize(24);
  m_TimerText->SetAnchor(EUIAnchor::TopLeft);
  m_TimerText->SetAnchoredPosition({100.0f, 100.0f});
  m_TimerText->RegisterComponent();

  m_ItemIcon = NewObject<MSpriteComponent>(this);
  m_ItemIcon->SetRenderSettings(10, RenderSpace::Screen);
  m_ItemIcon->SetRelativeLocation({80.0f, 900.0f});
  int handle = ResourceManager::GetInstance().LoadResourceGraph("Resources/images/speedup2.png");
  m_ItemIcon->SubmitGraph(handle, FScale(0.3f), 200);
  m_ItemIcon->SetVisibility(false);
  m_ItemIcon->RegisterComponent();

  m_ItemText = NewObject<UITextComponent>(this);
  m_ItemText->SetText("キノコ");
  m_ItemText->SetColor(0xFFFFFF);
  m_ItemText->SetFontSize(22);
  m_ItemText->SetAnchor(EUIAnchor::BottomLeft);
  m_ItemText->SetPivot({0.0f, 1.0f});
  m_ItemText->SetAnchoredPosition({55.0f, -20.0f});
  m_ItemText->SetVisibility(false);
  m_ItemText->RegisterComponent();

  m_LapText = NewObject<UITextComponent>(this);
  m_LapText->SetText("LAP 1 / 3");
  m_LapText->SetColor(0xFFFFFF);
  m_LapText->SetFontSize(28);
  m_LapText->SetAnchor(EUIAnchor::TopRight);
  m_LapText->SetAnchoredPosition({-120.0f, 60.0f});
  m_LapText->RegisterComponent();
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
