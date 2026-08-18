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
  DEFINE_ACTOR_CLASS(WClearHUD)
  WClearHUD();

  void SetClearTime(float ClearTime);
  void SetMultiplayerResults(const std::vector<FResultEntryViewData>& Results);
  void SetWorldRanking(const FWorldRankingBatchResult& Result, const std::string& IdentityKey);
  void SetWorldRankingPending();
  void SetWorldRankingError();
  void SetHostMode(bool bIsHost);
  void SetWaitingForHost(bool bWaiting);
  void SetWaitingForResults(bool bWaiting);
  void SetReturnCountdown(int Seconds);

  std::function<void()> OnBackToLobby;

 private:
  void ClearLobbyEntries();
  void ClearWorldEntries();
  void AddWorldSeparator();
  void RebuildActionNavigation();

  MUIVerticalBoxComponent* LobbyResultListBox = nullptr;
  MUIVerticalBoxComponent* WorldResultListBox = nullptr;
  MUIVerticalBoxComponent* ActionBox = nullptr;
  UIBoxButtonComponent* BackToLobbyButton = nullptr;
  UITextComponent* ClearTimeText = nullptr;
  UITextComponent* LobbyLoadingText = nullptr;
  UITextComponent* WorldStatusText = nullptr;
  UITextComponent* WaitingHostText = nullptr;
  UITextComponent* ReturnCountdownText = nullptr;
  std::vector<WRankEntryComponent*> LobbyEntryWidgets;
  std::vector<WRankEntryComponent*> WorldEntryWidgets;
  std::vector<UITextComponent*> WorldSeparatorWidgets;
};
