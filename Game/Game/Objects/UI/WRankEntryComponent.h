#pragma once
#include <UIWidgetComponent.h>
#include <string>

class UITextComponent;
class MSpriteComponent;
class WRankEntryComponent : public MUIWidgetComponent
{
public:
	WRankEntryComponent(int rank, const std::string& userId, float score, const std::string& deltaTimestamp);

	void RegisterComponent() override;

private:
	int m_Rank;
	std::string m_UserId;
	float m_Score;
	std::string m_DeltaTimestamp;

	UITextComponent* m_RankTextComponent = nullptr;
	UITextComponent* m_NameTextComponent = nullptr;
	UITextComponent* m_ScoreTextComponent = nullptr;
	UITextComponent* m_TimeTextComponent = nullptr;
	MSpriteComponent* m_SpriteComponent = nullptr;
};
