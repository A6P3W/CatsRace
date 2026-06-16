#include "WMainHUD.h"
#include <UITextComponent.h>
#include <iomanip>
#include <sstream>

WMainHUD::WMainHUD()
{
	auto text = std::make_unique<UITextComponent>("Time: 0.00", 0x00FF88, 24);
	m_TimerText = text.get();
	m_TimerText->SetAnchor(EUIAnchor::TopLeft);
	m_TimerText->SetAnchoredPosition({ 100.0f, 100.0f });
	AddComponent(std::move(text));
}

void WMainHUD::UpdateTimerText(float elapsedTime)
{
	std::ostringstream oss;
	oss << "Time: " << std::fixed << std::setprecision(2) << elapsedTime;
	m_TimerText->SetText(oss.str());
}
