#pragma once
#include <WidgetBase.h>
#include <string>
#include <functional>

enum class ENameSelectResult { UseExisting, EnterNew, Skip };
using FOnNameSelectResult = std::function<void(ENameSelectResult)>;

class UITextComponent;
class UIBoxButtonComponent;
class MSpriteComponent;

class WNameSelectDialog : public AWidgetBase
{
public:
	DEFINE_ACTOR_CLASS(WNameSelectDialog);
	WNameSelectDialog();

	void SetExistingName(const std::string& name);
	void SetOnResult(FOnNameSelectResult callback);

protected:
	void BeginPlay() override;

private:
	std::string m_ExistingName = "";
	FOnNameSelectResult m_Callback;

	MSpriteComponent* m_BgPanel = nullptr;
	UITextComponent* m_TitleText = nullptr;

	UIBoxButtonComponent* m_BtnUseExisting = nullptr;
	UITextComponent* m_TxtUseExisting = nullptr;

	UIBoxButtonComponent* m_BtnEnterNew = nullptr;
	UITextComponent* m_TxtEnterNew = nullptr;

	UIBoxButtonComponent* m_BtnSkip = nullptr;
	UITextComponent* m_TxtSkip = nullptr;
};
