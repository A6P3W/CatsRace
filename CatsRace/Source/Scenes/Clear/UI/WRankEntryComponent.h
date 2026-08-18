#pragma once
#include <UIWidgetComponent.h>

#include <string>

class UITextComponent;
class MSpriteComponent;
class WRankEntryComponent : public MUIWidgetComponent {
 public:
  WRankEntryComponent() = default;
  void InitializeWorld(int Rank, const std::string& PlayerName, float Score, bool bIsSelf);
  void Initialize(
      int rank, const std::string& playerName, bool bFinished, float finishTime, bool bLocalPlayer
  );

  void OnRegister() override;

 private:
  int m_Rank = 0;
  std::string m_UserId;
  float m_Score = 0.0f;
  bool m_bFinished = true;
  bool m_bLocalPlayer = false;

  UITextComponent* m_RankTextComponent = nullptr;
  UITextComponent* m_NameTextComponent = nullptr;
  UITextComponent* m_ScoreTextComponent = nullptr;
  UITextComponent* m_TimeTextComponent = nullptr;
  MSpriteComponent* m_SpriteComponent = nullptr;
};
