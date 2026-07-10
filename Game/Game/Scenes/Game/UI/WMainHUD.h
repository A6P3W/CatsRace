#pragma once
#include <WidgetBase.h>
#include <SpriteComponent.h>
class UITextComponent;
class MSpriteComponent;
class WMainHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WMainHUD);
  WMainHUD();
  void UpdateTimerText(float elapsedTime);
  void SetHeldItemVisible(bool bVisible);
  void UpdateLapText(int currentLap, int totalLaps);
 private:
  UITextComponent* m_TimerText = nullptr;
  MSpriteComponent* m_ItemIcon = nullptr;  
  UITextComponent* m_ItemText = nullptr;   
  UITextComponent* m_LapText = nullptr;
};
