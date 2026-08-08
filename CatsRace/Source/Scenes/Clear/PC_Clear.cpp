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

