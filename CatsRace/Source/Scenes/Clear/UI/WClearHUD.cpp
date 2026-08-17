#include "WClearHUD.h"

#include <UIBoxButton.h>
#include <UITextComponent.h>
#include <UIVerticalBoxComponent.h>

#include <iomanip>
#include <sstream>

#include "WRankEntryComponent.h"

namespace {
constexpr float ResultListWidth = 620.0f;
constexpr float ActionButtonWidth = 260.0f;
constexpr float ActionButtonHeight = 52.0f;
constexpr FColor ButtonNormalColor{32, 38, 48};
constexpr FColor ButtonHoveredColor{30, 115, 190};
constexpr FColor ButtonPressedColor{15, 78, 140};

UITextComponent* AddHeading(WClearHUD* Owner, const std::string& Text, float X) {
  auto* Heading = NewObject<UITextComponent>(Owner);
  Heading->SetText(Text);
  Heading->SetColor(FColor{255, 232, 92});
  Heading->SetFontSize(38);
  Heading->SetAnchor(EUIAnchor::TopCenter);
  Heading->SetPivot({0.5f, 0.5f});
  Heading->SetAnchoredPosition({X, 235.0f});
  Heading->RegisterComponent();
  return Heading;
}

UIBoxButtonComponent* AddActionButton(
    WClearHUD* Owner, MUIVerticalBoxComponent* Container, const std::string& Label
) {
  auto* Button = NewObject<UIBoxButtonComponent>(Owner);
  Button->SetSize(ActionButtonWidth, ActionButtonHeight);
  Button->SetColors(ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor);
  Button->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(Button);
  }
  Button->RegisterComponent();

  auto* ButtonLabel = NewObject<UITextComponent>(Owner);
  ButtonLabel->SetText(Label);
  ButtonLabel->SetColor(FColor{255, 255, 255});
  ButtonLabel->SetFontSize(33);
  ButtonLabel->AttachToComponent(Button);
  ButtonLabel->SetWidgetSize({ActionButtonWidth, ActionButtonHeight});
  ButtonLabel->SetAnchor(EUIAnchor::MiddleCenter);
  ButtonLabel->SetPivot({0.5f, 0.5f});
  ButtonLabel->RegisterComponent();
  return Button;
}
}  // namespace

WClearHUD::WClearHUD() {
  ClearTimeText = NewObject<UITextComponent>(this);
  ClearTimeText->SetText("Clear Time: --.--");
  ClearTimeText->SetColor(FColor{255, 255, 0});
  ClearTimeText->SetFontSize(54);
  ClearTimeText->SetAnchor(EUIAnchor::TopCenter);
  ClearTimeText->SetPivot({0.5f, 0.5f});
  ClearTimeText->SetAnchoredPosition({0.0f, 145.0f});
  ClearTimeText->RegisterComponent();

  AddHeading(this, "Lobby Results", -360.0f);
  AddHeading(this, "World Ranking", 360.0f);

  LobbyResultListBox = NewObject<MUIVerticalBoxComponent>(this);
  LobbyResultListBox->SetAnchor(EUIAnchor::TopCenter);
  LobbyResultListBox->SetPivot({0.5f, 0.0f});
  LobbyResultListBox->SetWidgetSize({ResultListWidth, 1.0f});
  LobbyResultListBox->SetAnchoredPosition({-360.0f, 285.0f});
  LobbyResultListBox->SetSpacing(7.0f);
  LobbyResultListBox->SetAutoResize(true);
  LobbyResultListBox->RegisterComponent();

  WorldResultListBox = NewObject<MUIVerticalBoxComponent>(this);
  WorldResultListBox->SetAnchor(EUIAnchor::TopCenter);
  WorldResultListBox->SetPivot({0.5f, 0.0f});
  WorldResultListBox->SetWidgetSize({ResultListWidth, 1.0f});
  WorldResultListBox->SetAnchoredPosition({360.0f, 285.0f});
  WorldResultListBox->SetSpacing(5.0f);
  WorldResultListBox->SetAutoResize(true);
  WorldResultListBox->RegisterComponent();

  LobbyLoadingText = NewObject<UITextComponent>(this);
  LobbyLoadingText->SetText("Waiting for results...");
  LobbyLoadingText->SetColor(FColor{136, 136, 136});
  LobbyLoadingText->SetFontSize(32);
  LobbyLoadingText->SetAnchor(EUIAnchor::TopCenter);
  LobbyLoadingText->SetPivot({0.5f, 0.5f});
  LobbyLoadingText->SetAnchoredPosition({-360.0f, 300.0f});
  LobbyLoadingText->RegisterComponent();

  WorldStatusText = NewObject<UITextComponent>(this);
  WorldStatusText->SetText("World Ranking: 集計中...");
  WorldStatusText->SetColor(FColor{136, 136, 136});
  WorldStatusText->SetFontSize(32);
  WorldStatusText->SetAnchor(EUIAnchor::TopCenter);
  WorldStatusText->SetPivot({0.5f, 0.5f});
  WorldStatusText->SetAnchoredPosition({360.0f, 300.0f});
  WorldStatusText->RegisterComponent();

  ReturnCountdownText = NewObject<UITextComponent>(this);
  ReturnCountdownText->SetColor(FColor{221, 221, 221});
  ReturnCountdownText->SetFontSize(32);
  ReturnCountdownText->SetAnchor(EUIAnchor::BottomCenter);
  ReturnCountdownText->SetPivot({0.5f, 1.0f});
  ReturnCountdownText->SetAnchoredPosition({0.0f, -125.0f});
  ReturnCountdownText->SetVisibility(false);
  ReturnCountdownText->RegisterComponent();

  ActionBox = NewObject<MUIVerticalBoxComponent>(this);
  ActionBox->SetAnchor(EUIAnchor::BottomCenter);
  ActionBox->SetPivot({0.5f, 1.0f});
  ActionBox->SetWidgetSize({ActionButtonWidth, 1.0f});
  ActionBox->SetAnchoredPosition({0.0f, -55.0f});
  ActionBox->RegisterComponent();
  BackToLobbyButton = AddActionButton(this, ActionBox, "Back To Lobby");

  WaitingHostText = NewObject<UITextComponent>(this);
  WaitingHostText->SetText("Waiting for host.");
  WaitingHostText->SetColor(FColor{221, 221, 221});
  WaitingHostText->SetFontSize(32);
  WaitingHostText->SetAnchor(EUIAnchor::BottomCenter);
  WaitingHostText->SetPivot({0.5f, 1.0f});
  WaitingHostText->SetAnchoredPosition({0.0f, -70.0f});
  WaitingHostText->SetVisibility(false);
  WaitingHostText->RegisterComponent();

  BackToLobbyButton->SetOnPressed([this]() {
    if (OnBackToLobby) {
      OnBackToLobby();
    }
  });
  RebuildActionNavigation();
}

void WClearHUD::SetClearTime(float ClearTime) {
  if (!ClearTimeText) {
    return;
  }
  if (ClearTime < 0.0f) {
    ClearTimeText->SetText("Clear Time: --.--");
    return;
  }
  std::ostringstream Text;
  Text << "Clear Time: " << std::fixed << std::setprecision(2) << ClearTime;
  ClearTimeText->SetText(Text.str());
}

void WClearHUD::SetMultiplayerResults(const std::vector<FResultEntryViewData>& Results) {
  SetWaitingForResults(Results.empty());
  ClearLobbyEntries();
  int Rank = 1;
  for (const auto& Result : Results) {
    auto* Entry = NewObject<WRankEntryComponent>(this);
    Entry->Initialize(
        Result.bFinished ? Rank++ : 0,
        Result.PlayerName,
        Result.bFinished,
        Result.FinishTime,
        Result.bLocalPlayer
    );
    if (LobbyResultListBox) {
      LobbyResultListBox->AddItem(Entry);
    }
    LobbyEntryWidgets.push_back(Entry);
    Entry->RegisterComponent();
  }
}

void WClearHUD::SetWorldRanking(
    const FWorldRankingBatchResult& Result, const std::string& IdentityKey
) {
  ClearWorldEntries();
  WorldStatusText->SetVisibility(false);
  AddWorldSectionLabel("Top 5");
  for (const auto& RankingEntry : Result.Top) {
    auto* Entry = NewObject<WRankEntryComponent>(this);
    Entry->InitializeWorld(
        RankingEntry.Rank,
        RankingEntry.PlayerName,
        RankingEntry.Score,
        RankingEntry.bIsSelf || RankingEntry.IdentityKey == IdentityKey
    );
    WorldResultListBox->AddItem(Entry);
    WorldEntryWidgets.push_back(Entry);
    Entry->RegisterComponent();
  }

  AddWorldSectionLabel("Self ±2");
  const auto RankingIt = Result.RankingsByIdentity.find(IdentityKey);
  if (RankingIt != Result.RankingsByIdentity.end()) {
    for (const auto& RankingEntry : RankingIt->second.AroundSelf) {
      auto* Entry = NewObject<WRankEntryComponent>(this);
      Entry->InitializeWorld(
          RankingEntry.Rank,
          RankingEntry.PlayerName,
          RankingEntry.Score,
          RankingEntry.bIsSelf || RankingEntry.IdentityKey == IdentityKey
      );
      WorldResultListBox->AddItem(Entry);
      WorldEntryWidgets.push_back(Entry);
      Entry->RegisterComponent();
    }
  }
}

void WClearHUD::SetWorldRankingPending() {
  ClearWorldEntries();
  WorldStatusText->SetText("World Ranking: 集計中...");
  WorldStatusText->SetVisibility(true);
}

void WClearHUD::SetWorldRankingError() {
  ClearWorldEntries();
  WorldStatusText->SetText("World Ranking: 取得失敗");
  WorldStatusText->SetVisibility(true);
}

void WClearHUD::SetHostMode(bool bIsHost) {
  BackToLobbyButton->SetVisibility(bIsHost);
  ActionBox->MarkLayoutDirty();
  RebuildActionNavigation();
  SetFocusedButton(bIsHost ? BackToLobbyButton : nullptr);
}

void WClearHUD::SetWaitingForHost(bool bWaiting) { WaitingHostText->SetVisibility(bWaiting); }

void WClearHUD::SetWaitingForResults(bool bWaiting) { LobbyLoadingText->SetVisibility(bWaiting); }

void WClearHUD::SetReturnCountdown(int Seconds) {
  ReturnCountdownText->SetVisibility(Seconds >= 0);
  if (Seconds > 0) {
    ReturnCountdownText->SetText("Returning to lobby in " + std::to_string(Seconds) + "...");
  } else if (Seconds == 0) {
    ReturnCountdownText->SetText("Returning to lobby...");
  }
}

void WClearHUD::ClearLobbyEntries() {
  LobbyResultListBox->ClearItems();
  for (auto* Entry : LobbyEntryWidgets) {
    if (Entry) {
      Entry->DestroyComponent();
    }
  }
  LobbyEntryWidgets.clear();
}

void WClearHUD::ClearWorldEntries() {
  WorldResultListBox->ClearItems();
  for (auto* Entry : WorldEntryWidgets) {
    if (Entry) {
      Entry->DestroyComponent();
    }
  }
  for (auto* Label : WorldSectionLabels) {
    if (Label) {
      Label->DestroyComponent();
    }
  }
  WorldEntryWidgets.clear();
  WorldSectionLabels.clear();
}

void WClearHUD::AddWorldSectionLabel(const std::string& Label) {
  auto* Text = NewObject<UITextComponent>(this);
  Text->SetText(Label);
  Text->SetColor(FColor{126, 203, 255});
  Text->SetFontSize(27);
  Text->SetWidgetSize({ResultListWidth, 32.0f});
  WorldResultListBox->AddItem(Text);
  WorldSectionLabels.push_back(Text);
  Text->RegisterComponent();
}

void WClearHUD::RebuildActionNavigation() { ActionBox->BuildNavigation(); }
