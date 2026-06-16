#include "WCountDown.h"
#include <UITextComponent.h>

WCountDown::WCountDown()
{
	auto text = std::make_unique<UITextComponent>("", 0xFFFFFF, 240);
	m_Text = text.get();
	m_Text->SetAnchor(EUIAnchor::MiddleCenter);
	AddComponent(std::move(text));
}

void WCountDown::SetCountText(const std::string& text)
{
	m_Text->SetText(text);
}
