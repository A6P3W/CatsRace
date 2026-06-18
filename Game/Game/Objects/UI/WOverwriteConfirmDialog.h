#pragma once
#include <WidgetBase.h>
#include <string>
#include <functional>

enum class EOverwriteResult { Overwrite, ReEnter };
using FOnOverwriteResult = std::function<void(EOverwriteResult)>;

class UITextComponent;
class UIBoxButtonComponent;
class MSpriteComponent;

class WOverwriteConfirmDialog : public AWidgetBase
{
public:
	DEFINE_ACTOR_CLASS(WOverwriteConfirmDialog);
	WOverwriteConfirmDialog();

	void SetOnResult(FOnOverwriteResult callback);

protected:
	void BeginPlay() override;

private:
	FOnOverwriteResult m_Callback;

	MSpriteComponent* m_BgPanel = nullptr;
	UITextComponent* m_TitleText = nullptr;
	UITextComponent* m_SubText = nullptr;

	UIBoxButtonComponent* m_BtnOverwrite = nullptr;
	UITextComponent* m_TxtOverwrite = nullptr;

	UIBoxButtonComponent* m_BtnReEnter = nullptr;
	UITextComponent* m_TxtReEnter = nullptr;
};
