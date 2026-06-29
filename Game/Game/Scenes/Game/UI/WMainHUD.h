#pragma once
#include <WidgetBase.h>

class UITextComponent;

class WMainHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WMainHUD);
  WMainHUD();
  void UpdateTimerText(float elapsedTime);

 private:
  UITextComponent* m_TimerText = nullptr;
};
