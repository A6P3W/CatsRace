#include "WRankEntryComponent.h"

#include <Actor.h>
#include <SpriteComponent.h>
#include <UITextComponent.h>

#include <iomanip>
#include <sstream>

void WRankEntryComponent::InitializeWorld(
    int Rank, const std::string& PlayerName, float Score, bool bIsSelf
) {
  m_Rank = Rank;
  m_UserId = PlayerName;
  m_Score = Score;
  m_bFinished = true;
  m_bLocalPlayer = bIsSelf;
  SetWidgetSize({620.0f, 38.0f});
}

void WRankEntryComponent::Initialize(
    int rank, const std::string& playerName, bool bFinished, float finishTime, bool bLocalPlayer
) {
  m_Rank = rank;
  m_UserId = playerName;
  m_Score = finishTime;
  m_bFinished = bFinished;
  m_bLocalPlayer = bLocalPlayer;
  SetWidgetSize({620.0f, 40.0f});
}

void WRankEntryComponent::OnRegister() {
  MUIWidgetComponent::OnRegister();

  if (auto* owner = GetOwner()) {
    const float width = GetWidgetSize().X;
    const float height = GetWidgetSize().Y;
    const FColor textColor = m_bLocalPlayer ? FColor{255, 232, 92} : FColor{255, 255, 255};
    const FColor boxColor = m_bLocalPlayer ? FColor{54, 64, 88} : FColor{32, 36, 44};

    // 1. 順位テキストの生成
    std::string rankStr = "-";
    if (m_bFinished && m_Rank > 0) {
      rankStr = std::to_string(m_Rank);
    }
    m_RankTextComponent = NewObject<UITextComponent>(owner);
    m_RankTextComponent->SetText(rankStr);
    m_RankTextComponent->SetColor(textColor);
    m_RankTextComponent->SetFontSize(36);
    m_RankTextComponent->AttachToComponent(this);
    m_RankTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_RankTextComponent->SetPivot({0.0f, 0.5f});
    m_RankTextComponent->SetAnchoredPosition({-width * 0.5f + 24.0f, 0.0f});
    m_RankTextComponent->RegisterComponent();

    // 2. 名前テキストの生成
    m_NameTextComponent = NewObject<UITextComponent>(owner);
    m_NameTextComponent->SetText(m_UserId);
    m_NameTextComponent->SetColor(textColor);
    m_NameTextComponent->SetFontSize(36);
    m_NameTextComponent->AttachToComponent(this);
    m_NameTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_NameTextComponent->SetPivot({0.0f, 0.5f});
    m_NameTextComponent->SetAnchoredPosition({-width * 0.5f + 110.0f, 0.0f});
    m_NameTextComponent->RegisterComponent();

    // 3. スコアテキストの生成
    std::ostringstream oss;
    if (m_bFinished) {
      oss << std::fixed << std::setprecision(2) << m_Score;
    } else {
      oss << "DNF";
    }
    m_ScoreTextComponent = NewObject<UITextComponent>(owner);
    m_ScoreTextComponent->SetText(oss.str());
    m_ScoreTextComponent->SetColor(textColor);
    m_ScoreTextComponent->SetFontSize(36);
    m_ScoreTextComponent->AttachToComponent(this);
    m_ScoreTextComponent->SetAnchor(EUIAnchor::MiddleCenter);
    m_ScoreTextComponent->SetPivot({1.0f, 0.5f});
    m_ScoreTextComponent->SetAnchoredPosition({width * 0.5f - 24.0f, 0.0f});
    m_ScoreTextComponent->RegisterComponent();

    m_SpriteComponent = NewObject<MSpriteComponent>(owner);
    m_SpriteComponent->SetRenderSettings(GetFinalPriority(), RenderSpace::Screen);
    m_SpriteComponent->SubmitBox(
        width, height, FColor{boxColor.R, boxColor.G, boxColor.B, 96}, true
    );
    m_SpriteComponent->AttachToComponent(this);
    m_SpriteComponent->RegisterComponent();
  }
}
