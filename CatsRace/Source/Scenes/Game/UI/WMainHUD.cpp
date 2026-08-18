#include "WMainHUD.h"

#include <SpriteComponent.h>
#include <UITextComponent.h>

#include <iomanip>
#include <sstream>

#include "ResourceManager.h"

WMainHUD::WMainHUD() {
  m_TimerText = NewObject<UITextComponent>(this);
  m_TimerText->SetText("Time: 0.00");
  m_TimerText->SetColor(FColor{255, 255, 255});
  m_TimerText->SetFontSize(84);
  m_TimerText->SetAnchor(EUIAnchor::TopLeft);
  m_TimerText->SetAnchoredPosition({200.0f, 60.0f});
  m_TimerText->RegisterComponent();

  m_ItemIcon = NewObject<MSpriteComponent>(this);
  m_ItemIcon->SetRenderSettings(10, RenderSpace::Screen);
  m_ItemIcon->SetRelativeLocation({966.0f, 50.0f});
  int handle = ResourceManager::GetInstance().LoadResourceGraph("/Game/images/speedup2.png");
  m_ItemIcon->SubmitGraph(handle, FScale(0.6f), 200);
  m_ItemIcon->SetVisibility(false);
  m_ItemIcon->RegisterComponent();
  m_ItemText = NewObject<UITextComponent>(this);
  m_ItemText->SetColor(FColor{255, 255, 255});
  m_ItemText->SetFontSize(33);
  m_ItemText->SetAnchor(EUIAnchor::BottomLeft);
  m_ItemText->SetPivot({0.0f, 1.0f});
  m_ItemText->SetAnchoredPosition({55.0f, -20.0f});
  m_ItemText->SetVisibility(false);
  m_ItemText->RegisterComponent();

  m_LapText = NewObject<UITextComponent>(this);
  m_LapText->SetText("LAP 1 / 3");
  m_LapText->SetColor(FColor{255, 255, 255});
  m_LapText->SetFontSize(84);
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
  if (!m_LapText) return;

  if (currentLap >= totalLaps) {
    m_LapText->SetText("GOAL!");
  } else {
    m_LapText->SetText("LAP " + std::to_string(currentLap + 1) + " / " + std::to_string(totalLaps));
  }
}
void WMainHUD::SetLapVisible(bool bVisible) {
  if (m_LapText) m_LapText->SetVisibility(bVisible);
}
