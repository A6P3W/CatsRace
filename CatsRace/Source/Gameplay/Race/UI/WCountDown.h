#pragma once
#include <WidgetBase.h>

#include <string>

class UITextComponent;

class WCountDown : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WCountDown);
  WCountDown();
  void SetCountText(const std::string& text);

 private:
  UITextComponent* m_Text = nullptr;
};
