#include "Scenes/Clear/PC_Clear.h"

#include <DxLib.h>
#include <EnhancedInputComponent.h>
#include <KeyboardDevice.h>
#include <NetworkManager.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Core/MapData.h"
#include "InputManager.h"
#include "Log.h"
#include "SceneManager.h"
#include "Scenes/Clear/UI/WClearHUD.h"
#include "Scenes/Clear/UI/WNameInputDialog.h"
#include "Scenes/Clear/UI/WNameSelectDialog.h"
#include "Scenes/Clear/UI/WOverwriteConfirmDialog.h"
#include "Scenes/Clear/UI/WPostGameDialog.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Services/LeaderBoardManager.h"
#include "UIManager.h"
#include "World.h"

namespace {
enum : FNetworkRPCId { RPC_ServerSubmitLocalResult = 1 };

std::string GetReplayLevelPath() {
  auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (gi && !gi->last_level_path.empty()) {
    return gi->last_level_path;
  }
  return AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
}
}  // namespace

REGISTER_ACTOR(PC_Clear)

PC_Clear::PC_Clear() {
  SetUpdateableAnytime(true);
  RegisterRPC(
      RPC_ServerSubmitLocalResult,
      ENetRPCType::Server,
      this,
      &PC_Clear::Server_SubmitLocalResult
  );
}

void PC_Clear::BeginPlay() {
  APlayerController::BeginPlay();

  if (!bIsLocallyControlled) {
    return;
  }

  SetInputMode(EInputMode::UIOnly);
  SetupInputMappings();

  auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  m_ClearHUD = GetWorld()->SpawnActor<WClearHUD>();
  UIManager::GetInstance()->AddWidget(m_ClearHUD);
  UIManager::GetInstance()->SetFocusedWidget(m_ClearHUD);

  if (m_ClearHUD) {
    const bool bIsStandalone = GetWorld()->IsStandalone();
    m_ClearHUD->SetHostMode(false);
    m_ClearHUD->SetWaitingForHost(false);

    if (bIsStandalone) {
      m_ClearHUD->SetClearTime(gi ? gi->ClearTime : -1.0f);
    } else {
      m_ClearHUD->SetClearTime(-1.0f);
      m_ClearHUD->SetWaitingForResults(true);
      m_ClearHUD->SetReturnCountdown(10);
    }
  }

  if (GetWorld()->IsStandalone()) {
    // Fetch and display leaderboard initially
    FetchAndDisplay();

    // Start name registration flow
    ShowNameFlow();
  }
}

void PC_Clear::OnUpdate(float DeltaTime) {
  if (bIsLocallyControlled) {
    if (MEnhancedInputComponent* input = GetInputComponent()) {
      if (InputMapper* mapper = GetInputMapper()) {
        input->ProcessInputBindings(*mapper, true, false);
      }
    }
  }

  if (!bIsLocallyControlled || !m_ClearHUD || GetWorld()->IsStandalone()) {
    return;
  }

  m_ResultSubmitRetryCooldown =
      (std::max)(0.0f, m_ResultSubmitRetryCooldown - DeltaTime);

  DisplayReturnCountdownRemaining -= DeltaTime;
  const int displayedCountdown = (std::max)(0, static_cast<int>(std::ceil(DisplayReturnCountdownRemaining)));
  if (displayedCountdown != LastDisplayedReturnCountdown) {
    LastDisplayedReturnCountdown = displayedCountdown;
    m_ClearHUD->SetReturnCountdown(displayedCountdown);
  }

  SubmitLocalResultToServerIfNeeded();
  RefreshMultiplayerResults();
}

void PC_Clear::ShowNameFlow() {
  auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi) return;

  if (!gi->user_id.empty()) {
    m_NameSelectDialog = GetWorld()->SpawnActor<WNameSelectDialog>();
    m_NameSelectDialog->SetZOrderOffset(10);
    m_NameSelectDialog->SetExistingName(gi->user_id);
    m_NameSelectDialog->SetOnResult([this, gi](ENameSelectResult result) {
      if (m_NameSelectDialog) {
        m_NameSelectDialog->Destroy();
        m_NameSelectDialog = nullptr;
      }
      if (result == ENameSelectResult::UseExisting) {
        ExecutePostScore(gi->user_id);
      } else if (result == ENameSelectResult::EnterNew) {
        ShowNameInputDialog();
      } else if (result == ENameSelectResult::Skip) {
        ShowPostGameDialog();
      }
    });
    UIManager::GetInstance()->AddWidget(m_NameSelectDialog);
    UIManager::GetInstance()->SetFocusedWidget(m_NameSelectDialog);
  } else {
    ShowNameInputDialog();
  }
}

void PC_Clear::ShowNameInputDialog() {
  m_NameInputDialog = GetWorld()->SpawnActor<WNameInputDialog>();
  m_NameInputDialog->SetZOrderOffset(10);
  m_NameInputDialog->SetOnNameConfirmed([this](const std::string& name) {
    if (m_NameInputDialog) {
      m_NameInputDialog->Destroy();
      m_NameInputDialog = nullptr;
    }
    CheckDuplicateAndPost(name);
  });
  m_NameInputDialog->SetOnCancelled([this]() {
    if (m_NameInputDialog) {
      m_NameInputDialog->Destroy();
      m_NameInputDialog = nullptr;
    }
    ShowPostGameDialog();
  });
  UIManager::GetInstance()->AddWidget(m_NameInputDialog);
  UIManager::GetInstance()->SetFocusedWidget(m_NameInputDialog);
}

void PC_Clear::CheckDuplicateAndPost(const std::string& name) {
  bool isDuplicate = false;
  for (const auto& existingId : m_FetchedUserIds) {
    if (existingId == name) {
      isDuplicate = true;
      break;
    }
  }

  if (isDuplicate) {
    m_OverwriteDialog = GetWorld()->SpawnActor<WOverwriteConfirmDialog>();
    m_OverwriteDialog->SetZOrderOffset(10);
    m_OverwriteDialog->SetOnResult([this, name](EOverwriteResult result) {
      if (m_OverwriteDialog) {
        m_OverwriteDialog->Destroy();
        m_OverwriteDialog = nullptr;
      }
      if (result == EOverwriteResult::Overwrite) {
        if (auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
          gi->user_id = name;
        }
        ExecutePostScore(name);
      } else if (result == EOverwriteResult::ReEnter) {
        ShowNameInputDialog();
      }
    });
    UIManager::GetInstance()->AddWidget(m_OverwriteDialog);
    UIManager::GetInstance()->SetFocusedWidget(m_OverwriteDialog);
  } else {
    if (auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
      gi->user_id = name;
    }
    ExecutePostScore(name);
  }
}

void PC_Clear::ExecutePostScore(const std::string& name) {
  auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi) return;

  std::string map_id = gi->map_id;
  auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
  LBM->PostScore(map_id, name, [this](bool bSuccess) {
    if (!bSuccess) {
      if (m_ClearHUD) {
        m_ClearHUD->SetErrorText("Failed to Post Score");
      }
    }
    FetchAndDisplay();
    ShowPostGameDialog();
  });
}

void PC_Clear::FetchAndDisplay() {
  auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi) return;

  std::string map_id = gi->map_id;
  auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
  LBM->FetchLeaderBoard(
      map_id, [this](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
        if (!bSuccess) {
          if (m_ClearHUD) {
            m_ClearHUD->SetErrorText("Failed to Fetch LeaderBoard");
          }
          return;
        }
        m_FetchedUserIds.clear();
        for (const auto& entry : entries) {
          m_FetchedUserIds.push_back(entry.user_id);
        }
        if (m_ClearHUD) {
          m_ClearHUD->SetLeaderBoard(entries);
        }
      }
  );
}

void PC_Clear::ShowPostGameDialog() {
  m_PostGameDialog = GetWorld()->SpawnActor<WPostGameDialog>();
  m_PostGameDialog->SetZOrderOffset(10);
  m_PostGameDialog->SetOnResult([this](EPostGameResult result) {
    if (m_PostGameDialog) {
      m_PostGameDialog->Destroy();
      m_PostGameDialog = nullptr;
    }
    if (result == EPostGameResult::PlayAgain) {
      GetWorld()->ServerTravel(GetReplayLevelPath());
    } else if (result == EPostGameResult::BackToTitle) {
      GetWorld()->ServerTravel(GameSceneIds::Lobby);
    }
  });
  UIManager::GetInstance()->AddWidget(m_PostGameDialog);
  UIManager::GetInstance()->SetFocusedWidget(m_PostGameDialog);
}

std::vector<ALobbyPlayerState*> PC_Clear::GetResultStates() {
  std::map<FNetworkConnectionId, ALobbyPlayerState*> stateByConnectionId;
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return {};
  }
  for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    if (auto* state = dynamic_cast<ALobbyPlayerState*>(actorPtr.get())) {
      if (state->IsPendingDestroy()) {
        continue;
      }

      auto existingIt = stateByConnectionId.find(state->OwnerConnectionId);
      if (existingIt == stateByConnectionId.end()) {
        stateByConnectionId[state->OwnerConnectionId] = state;
        continue;
      }

      ALobbyPlayerState* existing = existingIt->second;
      const bool bPreferState =
          (state->bHasAuthority && existing && !existing->bHasAuthority) ||
          (state->IsFinished() && existing && !existing->IsFinished()) ||
          (state->IsFinished() && state->GetFinishTime() > 0.0f && existing &&
           existing->GetFinishTime() <= 0.0f);
      if (bPreferState) {
        existingIt->second = state;
      }
    }
  }

  std::vector<ALobbyPlayerState*> states;
  for (const auto& pair : stateByConnectionId) {
    if (pair.second) {
      states.push_back(pair.second);
    }
  }

  std::sort(
      states.begin(), states.end(), [](const ALobbyPlayerState* a, const ALobbyPlayerState* b) {
        if (a->IsFinished() != b->IsFinished()) return a->IsFinished() > b->IsFinished();
        if (a->IsFinished() && a->GetFinishTime() != b->GetFinishTime())
          return a->GetFinishTime() < b->GetFinishTime();
        return a->OwnerConnectionId < b->OwnerConnectionId;
      }
  );
  return states;
}

void PC_Clear::RefreshMultiplayerResults() {
  if (!m_ClearHUD) {
    return;
  }

  const FNetworkConnectionId localId = NetworkManager::GetInstance().GetLocalConnectionId();
  const auto states = GetResultStates();
  std::ostringstream signature;
  signature << std::fixed << std::setprecision(2);

  std::vector<FResultEntryViewData> results;
  results.reserve(states.size());

  for (const auto* state : states) {
    if (!state) {
      continue;
    }

    const bool bLocalPlayer =
        (GetWorld()->IsServer() && state->OwnerConnectionId == 0) ||
        state->OwnerConnectionId == localId || state->bIsLocallyControlled;

    FResultEntryViewData data;
    data.ConnectionId = state->OwnerConnectionId;
    data.PlayerName = state->GetPlayerName();
    data.bFinished = state->IsFinished();
    data.FinishTime = state->GetFinishTime();
    data.bLocalPlayer = bLocalPlayer;
    results.push_back(data);

    signature << data.ConnectionId << ':' << data.PlayerName << ':' << (data.bFinished ? 1 : 0)
              << ':' << data.FinishTime << '|';

    if (bLocalPlayer && data.bFinished) {
      m_ClearHUD->SetClearTime(data.FinishTime);
    }
  }

  m_ClearHUD->SetWaitingForResults(results.empty());
  m_ClearHUD->SetHostMode(false);
  m_ClearHUD->SetWaitingForHost(false);

  const std::string nextSignature = signature.str();
  if (nextSignature != m_LastResultSignature) {
    m_LastResultSignature = nextSignature;
    m_ClearHUD->SetMultiplayerResults(results);
  }
}

void PC_Clear::SubmitLocalResultToServerIfNeeded() {
  if (m_bSubmittedLocalResult || GetWorld()->IsServer() ||
      GetWorld()->IsStandalone()) {
    return;
  }

  auto* gi =
      dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi || gi->ClearTime < 0.0f) {
    return;
  }

  const FNetworkConnectionId localId =
      NetworkManager::GetInstance().GetLocalConnectionId();

  // サーバーから完走結果が返ってきた場合だけ送信完了とする
  for (const auto* state : GetResultStates()) {
    if (state && state->OwnerConnectionId == localId &&
        state->IsFinished()) {
      m_bSubmittedLocalResult = true;
      return;
    }
  }

  if (m_ResultSubmitRetryCooldown > 0.0f) {
    return;
  }

  const bool bSent = InvokeRPC(
      RPC_ServerSubmitLocalResult,
      ENetRPCType::Server,
      ENetPacketReliability::Reliable,
      gi->ClearTime
  );

  m_ResultSubmitRetryCooldown = bSent ? 0.5f : 0.1f;
}

void PC_Clear::Server_SubmitLocalResult(float FinishTime) {
  if (!GetWorld() || !GetWorld()->IsServer() || FinishTime < 0.0f) {
    return;
  }

  const FNetworkConnectionId connectionId = OwnerConnectionId;
  std::string playerName =
      "Player " + std::to_string(connectionId + 1);

  for (auto* state : GetResultStates()) {
    if (!state || state->OwnerConnectionId != connectionId) {
      continue;
    }

    playerName = state->GetPlayerName();

    if (!state->IsFinished() || state->GetFinishTime() <= 0.0f) {
      state->SetFinishResult(true, FinishTime);
    }
    break;
  }

  auto* gi =
      dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi) {
    return;
  }

  auto existing = std::find_if(
      gi->multiplayer_results.begin(),
      gi->multiplayer_results.end(),
      [connectionId](const GI_main::FMultiplayerResult& result) {
        return result.ConnectionId == connectionId;
      }
  );

  if (existing == gi->multiplayer_results.end()) {
    GI_main::FMultiplayerResult result;
    result.ConnectionId = connectionId;
    result.PlayerName = playerName;
    result.bFinished = true;
    result.FinishTime = FinishTime;
    gi->multiplayer_results.push_back(result);
  } else {
    existing->bFinished = true;
    existing->FinishTime = FinishTime;

    if (!playerName.empty()) {
      existing->PlayerName = playerName;
    }
  }

  M_LOG(
      "Clear result accepted: connection={}, time={}",
      connectionId,
      FinishTime
  );
}

