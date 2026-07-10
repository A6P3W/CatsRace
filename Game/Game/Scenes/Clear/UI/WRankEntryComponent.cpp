#include "WRankEntryComponent.h"

#include <Actor.h>
#include <DxLib.h>
#include <SpriteComponent.h>
#include <UITextComponent.h>

#include <iomanip>
#include <sstream>

WRankEntryComponent::WRankEntryComponent(
    int rank, const std::string& userId, float score, const std::string& deltaTimestamp
)
    : m_Rank(rank), m_UserId(userId), m_Score(score), m_DeltaTimestamp(deltaTimestamp) {
  SetWidgetSize({540.0f, 40.0f});
}

WRankEntryComponent::WRankEntryComponent(
    int rank,
    const std::string& playerName,
    bool bFinished,
    float finishTime,
    bool bLocalPlayer
)
    : m_Rank(rank),
      m_UserId(playerName),
      m_Score(finishTime),
      m_bShowTimestamp(false),
      m_bFinished(bFinished),
      m_bLocalPlayer(bLocalPlayer) {
  SetWidgetSize({620.0f, 40.0f});
}

void WRankEntryComponent::RegisterComponent() {
  MUIWidgetComponent::RegisterComponent();

  if (auto* owner = GetOwner()) {
    const float width = GetWidgetSize().X;
    const float height = GetWidgetSize().Y;
    const int textColor = m_bLocalPlayer ? GetColor(255, 232, 92) : 0xFFFFFF;
    const int boxColor = m_bLocalPlayer ? GetColor(54, 64, 88) : GetColor(32, 36, 44);

    // 1. 順位テキストの生成
    std::string rankStr = "-";
    if (m_bFinished && m_Rank > 0) {
      rankStr = m_bShowTimestamp ? "#" + std::to_string(m_Rank) : std::to_string(m_Rank);
    }
    auto rankComp = std::make_unique<UITextComponent>(rankStr, textColor, 24);
    m_RankTextComponent = rankComp.get();
    m_RankTextComponent->SetParentComponent(this);
    m_RankTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_RankTextComponent->SetPivot({0.0f, 0.5f});
    m_RankTextComponent->SetAnchoredPosition({-width * 0.5f + 24.0f, 0.0f});
    owner->AddComponent(std::move(rankComp));

    // 2. 名前テキストの生成
    auto nameComp = std::make_unique<UITextComponent>(m_UserId, textColor, 24);
    m_NameTextComponent = nameComp.get();
    m_NameTextComponent->SetParentComponent(this);
    m_NameTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_NameTextComponent->SetPivot({0.0f, 0.5f});
    m_NameTextComponent->SetAnchoredPosition({-width * 0.5f + 110.0f, 0.0f});
    owner->AddComponent(std::move(nameComp));

    // 3. 経過時間テキストの生成
    if (m_bShowTimestamp) {
      std::string timeStr = m_DeltaTimestamp;
      auto timeComp = std::make_unique<UITextComponent>(timeStr, textColor, 20);
      m_TimeTextComponent = timeComp.get();
      m_TimeTextComponent->SetParentComponent(this);
      m_TimeTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
      m_TimeTextComponent->SetPivot({1.0f, 0.5f});
      m_TimeTextComponent->SetAnchoredPosition({110.0f, 0.0f});
      owner->AddComponent(std::move(timeComp));
    }

    // 4. スコアテキストの生成
    std::ostringstream oss;
    if (m_bFinished) {
      oss << std::fixed << std::setprecision(2) << m_Score;
    } else {
      oss << "DNF";
    }
    auto scoreComp = std::make_unique<UITextComponent>(oss.str(), textColor, 24);
    m_ScoreTextComponent = scoreComp.get();
    m_ScoreTextComponent->SetParentComponent(this);
    m_ScoreTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_ScoreTextComponent->SetPivot({1.0f, 0.5f});
    m_ScoreTextComponent->SetAnchoredPosition({width * 0.5f - 24.0f, 0.0f});
    owner->AddComponent(std::move(scoreComp));

    auto spriteComp = std::make_unique<MSpriteComponent>(GetFinalPriority(), RenderSpace::Screen);
    m_SpriteComponent = spriteComp.get();
    m_SpriteComponent->SetRelativeLocation({-width * 0.5f, -height * 0.5f});
    m_SpriteComponent->SubmitBox(width, height, boxColor, true, 96);
    m_SpriteComponent->SetParentComponent(this);
    owner->AddComponent(std::move(spriteComp));
  }
}
