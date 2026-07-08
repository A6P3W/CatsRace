#pragma once
#include <WidgetBase.h>

#include <string>
#include <vector>

#include "Services/LeaderBoardManager.h"

class UITextComponent;
class WRankEntryComponent;

class WClearHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WClearHUD);
  WClearHUD();

  void SetClearTime(float clearTime);
  void SetLeaderBoard(const std::vector<FLeaderBoardEntry>& entries);

  void SetErrorText(std::string e);



 private:
  UITextComponent* m_ClearTimeText = nullptr;
  UITextComponent* m_LoadingText = nullptr;
  std::vector<WRankEntryComponent*> m_RankEntries;

  const float m_EntryStartY = 300.0f;
  const float m_EntryHeight = 45.0f;


};
