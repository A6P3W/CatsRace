#include "WRankEntryComponent.h"
#include <UITextComponent.h>
#include <SpriteComponent.h>
#include <Actor.h>
#include <sstream>
#include <iomanip>
#include <DxLib.h>
WRankEntryComponent::WRankEntryComponent(int rank, const std::string& userId, float score)
	: m_Rank(rank)
	, m_UserId(userId)
	, m_Score(score)
{
	SetWidgetSize({ 800.0f, 40.0f });
}

void WRankEntryComponent::RegisterComponent()
{
	MUIWidgetComponent::RegisterComponent();

	if (auto* owner = GetOwner()) {
		std::ostringstream oss;
		oss << "#" << m_Rank << "  " << m_UserId << "  " << std::fixed << std::setprecision(2) << m_Score;

		auto textComp = std::make_unique<UITextComponent>(oss.str(), 0xFFFFFF, 24);
		m_TextComponent = textComp.get();

		m_TextComponent->SetParentComponent(this);
		m_TextComponent->SetAnchor(EUIAnchor::MiddleCenter);
		m_TextComponent->SetPivot({ 0.5f, 0.5f });
		m_TextComponent->SetAnchoredPosition({ 0.0f, 0.0f });

		owner->AddComponent(std::move(textComp));

		auto spriteComp = std::make_unique<MSpriteComponent>(GetFinalPriority(), RenderSpace::Screen);
		m_SpriteComponent = spriteComp.get();
		m_SpriteComponent->SetRelativeLocation({ -800 * 0.5f, -40 * 0.5f });
		m_SpriteComponent->SubmitBox(800, 40, GetColor(255,255,255), true,100);
		m_SpriteComponent->SetParentComponent(this);
		owner->AddComponent(std::move(spriteComp));
	}
}
