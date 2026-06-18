#pragma once
#include <UIWidgetComponent.h>
#include <string>

class UITextComponent;
class MSpriteComponent;
class WRankEntryComponent : public MUIWidgetComponent
{
public:
	WRankEntryComponent(int rank, const std::string& userId, float score);

	void RegisterComponent() override;

private:
	int m_Rank;
	std::string m_UserId;
	float m_Score;

	UITextComponent* m_RankTextComponent = nullptr;
	UITextComponent* m_NameTextComponent = nullptr;
	UITextComponent* m_ScoreTextComponent = nullptr;
	MSpriteComponent* m_SpriteComponent = nullptr;
};
