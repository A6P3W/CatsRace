#include "WCountDown.h"

#include <UITextComponent.h>

WCountDown::WCountDown() {
  m_Text = NewObject<UITextComponent>(this);
  m_Text->SetText("");
  m_Text->SetColor(FColor{255, 255, 255});
  m_Text->SetFontSize(240);
  m_Text->SetAnchor(EUIAnchor::MiddleCenter);
  m_Text->RegisterComponent();
}

void WCountDown::SetCountText(const std::string& text) { m_Text->SetText(text); }
