#pragma once
#include <WidgetBase.h>
#include <string>
#include <functional>

using FOnNameConfirmed = std::function<void(const std::string& name)>;

class UITextComponent;
class UIBoxButtonComponent;
class MSpriteComponent;

class WNameInputDialog : public AWidgetBase
{
public:
	DEFINE_ACTOR_CLASS(WNameInputDialog);
	WNameInputDialog();
	virtual ~WNameInputDialog() override;

	void SetOnNameConfirmed(FOnNameConfirmed callback);
	void OnUpdate(float DeltaTime) override;

protected:
	void BeginPlay() override;

private:
	FOnNameConfirmed m_Callback;
	int m_InputHandle = -1;

	MSpriteComponent* m_BgPanel = nullptr;
	UITextComponent* m_TitleText = nullptr;
	MSpriteComponent* m_InputBoxBg = nullptr;
	UITextComponent* m_InputText = nullptr;

	UIBoxButtonComponent* m_BtnSubmit = nullptr;
	UITextComponent* m_TxtSubmit = nullptr;
};
