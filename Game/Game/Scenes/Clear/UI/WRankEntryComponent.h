#pragma once
#include <UIWidgetComponent.h>

#include <string>

class UITextComponent;
class MSpriteComponent;
class WRankEntryComponent : public MUIWidgetComponent {
 public:
  WRankEntryComponent(
      int rank, const std::string& userId, float score, const std::string& deltaTimestamp
  );
  WRankEntryComponent(
      int rank,
      const std::string& playerName,
      bool bFinished,
      float finishTime,
      bool bLocalPlayer
  );

  void RegisterComponent() override;

 private:
  int m_Rank;
  std::string m_UserId;
  float m_Score;
  std::string m_DeltaTimestamp;
  bool m_bShowTimestamp = true;
  bool m_bFinished = true;
  bool m_bLocalPlayer = false;

  UITextComponent* m_RankTextComponent = nullptr;
  UITextComponent* m_NameTextComponent = nullptr;
  UITextComponent* m_ScoreTextComponent = nullptr;
  UITextComponent* m_TimeTextComponent = nullptr;
  MSpriteComponent* m_SpriteComponent = nullptr;
};
