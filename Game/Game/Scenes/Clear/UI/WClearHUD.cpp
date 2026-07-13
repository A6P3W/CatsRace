#include "WClearHUD.h"

#include <UIBoxButton.h>
#include <UITextComponent.h>
#include <UIVerticalBoxComponent.h>

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include "WRankEntryComponent.h"

namespace {
constexpr float ResultListWidth = 620.0f;
constexpr float ActionButtonWidth = 220.0f;
constexpr float ActionButtonHeight = 52.0f;
constexpr int ButtonNormalColor = 0x202630;
constexpr int ButtonHoveredColor = 0x1E73BE;
constexpr int ButtonPressedColor = 0x0F4E8C;

UIBoxButtonComponent* AddActionButton(
    WClearHUD* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label
) {
  auto* buttonPtr = NewObject<UIBoxButtonComponent>(Owner);
  buttonPtr->SetSize(ActionButtonWidth, ActionButtonHeight);
  buttonPtr->SetColors(ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor);
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(buttonPtr);
  }
  buttonPtr->RegisterComponent();

  auto* label = NewObject<UITextComponent>(Owner);
  label->SetText(Label);
  label->SetColor(0xFFFFFF);
  label->SetFontSize(22);
  label->AttachToComponent(buttonPtr);
  label->SetWidgetSize({ActionButtonWidth, ActionButtonHeight});
  label->SetAnchor(EUIAnchor::MiddleCenter);
  label->SetPivot({0.5f, 0.5f});
  label->SetAnchoredPosition({0.0f, 0.0f});
  label->RegisterComponent();

  return buttonPtr;
}
}  // namespace

WClearHUD::WClearHUD() {
  m_ClearTimeText = NewObject<UITextComponent>(this);
  m_ClearTimeText->SetText("Clear Time: --.--");
  m_ClearTimeText->SetColor(0xFFFF00);
  m_ClearTimeText->SetFontSize(36);
  m_ClearTimeText->SetAnchor(EUIAnchor::TopCenter);
  m_ClearTimeText->SetPivot({0.5f, 0.5f});
  m_ClearTimeText->SetAnchoredPosition({0.0f, 150.0f});
  m_ClearTimeText->RegisterComponent();

  m_ResultListBox = NewObject<MUIVerticalBoxComponent>(this);
  m_ResultListBox->SetAnchor(EUIAnchor::TopCenter);
  m_ResultListBox->SetPivot({0.5f, 0.0f});
  m_ResultListBox->SetWidgetSize({ResultListWidth, 1.0f});
  m_ResultListBox->SetAnchoredPosition({0.0f, 240.0f});
  m_ResultListBox->SetSpacing(8.0f);
  m_ResultListBox->SetAutoResize(true);
  m_ResultListBox->RegisterComponent();

  m_LoadingText = NewObject<UITextComponent>(this);
  m_LoadingText->SetText("Waiting for results...");
  m_LoadingText->SetColor(0x888888);
  m_LoadingText->SetFontSize(24);
  m_LoadingText->SetAnchor(EUIAnchor::TopCenter);
  m_LoadingText->SetPivot({0.5f, 0.5f});
  m_LoadingText->SetAnchoredPosition({0.0f, 240.0f});
  m_LoadingText->RegisterComponent();

  m_ActionBox = NewObject<MUIVerticalBoxComponent>(this);
  m_ActionBox->SetAnchor(EUIAnchor::BottomCenter);
  m_ActionBox->SetPivot({0.5f, 1.0f});
  m_ActionBox->SetWidgetSize({ActionButtonWidth, 1.0f});
  m_ActionBox->SetAnchoredPosition({0.0f, -70.0f});
  m_ActionBox->SetSpacing(12.0f);
  m_ActionBox->RegisterComponent();

  m_ReplayButton = AddActionButton(this, m_ActionBox, "Replay");
  m_BackToLobbyButton = AddActionButton(this, m_ActionBox, "Back To Lobby");

  m_WaitingHostText = NewObject<UITextComponent>(this);
  m_WaitingHostText->SetText("Waiting for host.");
  m_WaitingHostText->SetColor(0xDDDDDD);
  m_WaitingHostText->SetFontSize(24);
  m_WaitingHostText->SetAnchor(EUIAnchor::BottomCenter);
  m_WaitingHostText->SetPivot({0.5f, 1.0f});
  m_WaitingHostText->SetAnchoredPosition({0.0f, -90.0f});
  m_WaitingHostText->SetVisibility(false);
  m_WaitingHostText->RegisterComponent();

  if (m_ReplayButton) {
    m_ReplayButton->SetOnPressed([this]() {
      if (OnReplay) {
        OnReplay();
      }
    });
  }
  if (m_BackToLobbyButton) {
    m_BackToLobbyButton->SetOnPressed([this]() {
      if (OnBackToLobby) {
        OnBackToLobby();
      }
    });
  }

  RebuildActionNavigation();
}

void WClearHUD::SetClearTime(float clearTime) {
  if (!m_ClearTimeText) {
    return;
  }

  if (clearTime < 0.0f) {
    m_ClearTimeText->SetText("Clear Time: --.--");
    return;
  }

  std::ostringstream oss;
  oss << "Clear Time: " << std::fixed << std::setprecision(2) << clearTime;
  m_ClearTimeText->SetText(oss.str());
}

void WClearHUD::SetLeaderBoard(const std::vector<FLeaderBoardEntry>& entries) {
  SetWaitingForResults(false);
  ClearResultEntries();

  size_t count = entries.size() > 15 ? 15 : entries.size();
  for (size_t i = 0; i < count; ++i) {
    const auto& entryData = entries[i];
    int rank = static_cast<int>(i + 1);

    auto* rankEntryPtr = NewObject<WRankEntryComponent>(this);
    rankEntryPtr->Initialize(
        rank, entryData.user_id, entryData.score, entryData.delta_timestamp
    );
    rankEntryPtr->SetPivot({0.5f, 0.5f});
    if (m_ResultListBox) {
      m_ResultListBox->AddItem(rankEntryPtr);
    }

    m_ResultEntryWidgets.push_back(rankEntryPtr);
    rankEntryPtr->RegisterComponent();
  }
}

void WClearHUD::SetMultiplayerResults(const std::vector<FResultEntryViewData>& Results) {
  SetWaitingForResults(Results.empty());
  ClearResultEntries();

  int rank = 1;
  for (const FResultEntryViewData& result : Results) {
    const int entryRank = result.bFinished ? rank++ : 0;
    auto* entryPtr = NewObject<WRankEntryComponent>(this);
    entryPtr->Initialize(
        entryRank,
        result.PlayerName,
        result.bFinished,
        result.FinishTime,
        result.bLocalPlayer
    );
    entryPtr->SetPivot({0.5f, 0.5f});
    if (m_ResultListBox) {
      m_ResultListBox->AddItem(entryPtr);
    }

    m_ResultEntryWidgets.push_back(entryPtr);
    entryPtr->RegisterComponent();
  }
}

void WClearHUD::SetHostMode(bool bIsHost) {
  if (m_ReplayButton) {
    m_ReplayButton->SetVisibility(bIsHost);
  }
  if (m_BackToLobbyButton) {
    m_BackToLobbyButton->SetVisibility(bIsHost);
  }
  if (m_ActionBox) {
    m_ActionBox->MarkLayoutDirty();
  }
  RebuildActionNavigation();

  if (bIsHost) {
    if (!GetFocusedButton()) {
      SetFocusedButton(m_ReplayButton);
    }
  } else if (GetFocusedButton() == m_ReplayButton || GetFocusedButton() == m_BackToLobbyButton) {
    SetFocusedButton(nullptr);
  }
}

void WClearHUD::SetWaitingForHost(bool bWaiting) {
  if (m_WaitingHostText) {
    m_WaitingHostText->SetVisibility(bWaiting);
  }
}

void WClearHUD::SetWaitingForResults(bool bWaiting) {
  if (m_LoadingText) {
    m_LoadingText->SetVisibility(bWaiting);
    if (bWaiting) {
      m_LoadingText->SetText("Waiting for results...");
    }
  }
}

void WClearHUD::SetErrorText(std::string e) {
  if (m_LoadingText) {
    m_LoadingText->SetText(e);
    m_LoadingText->SetVisibility(true);
  }
}

void WClearHUD::ClearResultEntries() {
  if (m_ResultListBox) {
    m_ResultListBox->ClearItems();
  }

  for (WRankEntryComponent* entry : m_ResultEntryWidgets) {
    if (entry) {
      entry->DestroyComponent();
    }
  }

  m_ResultEntryWidgets.clear();
}

void WClearHUD::RebuildActionNavigation() {
  if (m_ActionBox) {
    m_ActionBox->BuildNavigation();
  }
}
