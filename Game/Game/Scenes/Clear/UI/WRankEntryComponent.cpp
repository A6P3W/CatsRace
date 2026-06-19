#include "WRankEntryComponent.h"
#include <UITextComponent.h>
#include <SpriteComponent.h>
#include <Actor.h>
#include <sstream>
#include <iomanip>
#include <DxLib.h>

WRankEntryComponent::WRankEntryComponent(int rank, const std::string& userId, float score, const std::string& deltaTimestamp)
	: m_Rank(rank)
	, m_UserId(userId)
	, m_Score(score)
	, m_DeltaTimestamp(deltaTimestamp)
{
	SetWidgetSize({ 540.0f, 40.0f });
}

void WRankEntryComponent::RegisterComponent()
{
	MUIWidgetComponent::RegisterComponent();

	if (auto* owner = GetOwner()) {
		// 1. 順位テキストの生成
		std::string rankStr = "#" + std::to_string(m_Rank);
		auto rankComp = std::make_unique<UITextComponent>(rankStr, 0xFFFFFF, 24);
		m_RankTextComponent = rankComp.get();
		m_RankTextComponent->SetParentComponent(this);
		m_RankTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
		m_RankTextComponent->SetPivot({ 0.0f, 0.5f });
		m_RankTextComponent->SetAnchoredPosition({ -245.0f, 0.0f });
		owner->AddComponent(std::move(rankComp));

		// 2. 名前テキストの生成
		auto nameComp = std::make_unique<UITextComponent>(m_UserId, 0xFFFFFF, 24);
		m_NameTextComponent = nameComp.get();
		m_NameTextComponent->SetParentComponent(this);
		m_NameTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
		m_NameTextComponent->SetPivot({ 0.0f, 0.5f });
		m_NameTextComponent->SetAnchoredPosition({ -120.0f, 0.0f });
		owner->AddComponent(std::move(nameComp));

		// 3. 経過時間テキストの生成
		std::string timeStr = m_DeltaTimestamp;
		auto timeComp = std::make_unique<UITextComponent>(timeStr, 0xFFFFFF, 20);
		m_TimeTextComponent = timeComp.get();
		m_TimeTextComponent->SetParentComponent(this);
		m_TimeTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
		m_TimeTextComponent->SetPivot({ 1.0f, 0.5f });
		m_TimeTextComponent->SetAnchoredPosition({ 110.0f, 0.0f });
		owner->AddComponent(std::move(timeComp));

		// 4. スコアテキストの生成
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(2) << m_Score;
		auto scoreComp = std::make_unique<UITextComponent>(oss.str(), 0xFFFFFF, 24);
		m_ScoreTextComponent = scoreComp.get();
		m_ScoreTextComponent->SetParentComponent(this);
		m_ScoreTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
		m_ScoreTextComponent->SetPivot({ 0.0f, 0.5f });
		m_ScoreTextComponent->SetAnchoredPosition({ 215.0f, 0.0f });
		owner->AddComponent(std::move(scoreComp));

		auto spriteComp = std::make_unique<MSpriteComponent>(GetFinalPriority(), RenderSpace::Screen);
		m_SpriteComponent = spriteComp.get();
		m_SpriteComponent->SetRelativeLocation({ -540 * 0.5f, -40 * 0.5f });
		m_SpriteComponent->SubmitBox(540, 40, GetColor(255,255,255), true,100);
		m_SpriteComponent->SetParentComponent(this);
		owner->AddComponent(std::move(spriteComp));
	}
}
