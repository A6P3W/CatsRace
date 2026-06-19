#pragma once
#include <WidgetBase.h>

class UITextComponent;
class UIBoxButtonComponent;

class WTitleHUD : public AWidgetBase
{
public:
	DEFINE_ACTOR_CLASS(WTitleHUD);
	WTitleHUD();

protected:
	void BeginPlay() override;

private:
	UIBoxButtonComponent* m_BtnStart = nullptr;
	UITextComponent* m_TxtStart = nullptr;

	UIBoxButtonComponent* m_BtnQuit = nullptr;
	UITextComponent* m_TxtQuit = nullptr;
};
