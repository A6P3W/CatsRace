#pragma once
#include <NetworkTypes.h>
#include <WidgetBase.h>

#include <functional>
#include <string>
#include <vector>

#include "Services/LeaderBoardManager.h"

class MUIVerticalBoxComponent;
class UIBoxButtonComponent;
class UITextComponent;
class WRankEntryComponent;

struct FResultEntryViewData {
  FNetworkConnectionId ConnectionId = 0;
  std::string PlayerName;
  bool bFinished = false;
  float FinishTime = 0.0f;
  bool bLocalPlayer = false;
};

class WClearHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WClearHUD);
  WClearHUD();

  void SetClearTime(float clearTime);
  void SetLeaderBoard(const std::vector<FLeaderBoardEntry>& entries);
  void SetMultiplayerResults(const std::vector<FResultEntryViewData>& Results);
  void SetHostMode(bool bIsHost);
  void SetWaitingForHost(bool bWaiting);
  void SetWaitingForResults(bool bWaiting);

  void SetErrorText(std::string e);

  std::function<void()> OnReplay;
  std::function<void()> OnBackToLobby;

 private:
  void ClearResultEntries();
  void RebuildActionNavigation();

  MUIVerticalBoxComponent* m_ResultListBox = nullptr;
  MUIVerticalBoxComponent* m_ActionBox = nullptr;

  UIBoxButtonComponent* m_ReplayButton = nullptr;
  UIBoxButtonComponent* m_BackToLobbyButton = nullptr;

  UITextComponent* m_ClearTimeText = nullptr;
  UITextComponent* m_LoadingText = nullptr;
  UITextComponent* m_WaitingHostText = nullptr;

  std::vector<WRankEntryComponent*> m_ResultEntryWidgets;
  const float m_EntryStartY = 300.0f;
  const float m_EntryHeight = 45.0f;


};
