#include "WClearHUD.h"

#include <UITextComponent.h>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

#include "WRankEntryComponent.h"
WClearHUD::WClearHUD() {
  auto clearTimeText = std::make_unique<UITextComponent>("Clear Time: --.--", 0xFFFF00, 36);
  m_ClearTimeText = clearTimeText.get();
  m_ClearTimeText->SetAnchor(EUIAnchor::TopCenter);
  m_ClearTimeText->SetPivot({0.5f, 0.5f});
  m_ClearTimeText->SetAnchoredPosition({0.0f, 150.0f});
  AddComponent(std::move(clearTimeText));

  auto loadingText = std::make_unique<UITextComponent>("Loading...", 0x888888, 24);
  m_LoadingText = loadingText.get();
  m_LoadingText->SetAnchor(EUIAnchor::TopCenter);
  m_LoadingText->SetPivot({0.5f, 0.5f});
  m_LoadingText->SetAnchoredPosition({0.0f, m_EntryStartY});
  AddComponent(std::move(loadingText));
}

void WClearHUD::SetClearTime(float clearTime) {
  std::ostringstream oss;
  oss << "Clear Time: " << std::fixed << std::setprecision(2) << clearTime;
  if (m_ClearTimeText) {
    m_ClearTimeText->SetText(oss.str());
  }
}

void WClearHUD::SetLeaderBoard(const std::vector<FLeaderBoardEntry>& entries) {
  if (m_LoadingText) {
    m_LoadingText->SetVisibility(false);
  }

  for (auto* entry : m_RankEntries) {
    if (entry) {
      entry->DestroyComponent();
    }
  }
  m_RankEntries.clear();

  size_t count = entries.size() > 15 ? 15 : entries.size();
  for (size_t i = 0; i < count; ++i) {
    const auto& entryData = entries[i];
    int rank = static_cast<int>(i + 1);

    auto rankEntry = std::make_unique<WRankEntryComponent>(
        rank, entryData.user_id, entryData.score, entryData.delta_timestamp
    );
    WRankEntryComponent* rankEntryPtr = rankEntry.get();

    rankEntryPtr->SetAnchor(EUIAnchor::TopRight);
    rankEntryPtr->SetPivot({1.0f, 0.0f});
    rankEntryPtr->SetAnchoredPosition({-70.0f, m_EntryStartY + i * m_EntryHeight});

    m_RankEntries.push_back(rankEntryPtr);
    AddComponent(std::move(rankEntry));
  }
}

void WClearHUD::SetErrorText(std::string e) {
  if (m_LoadingText) {
    m_LoadingText->SetText(e);
  }
}
