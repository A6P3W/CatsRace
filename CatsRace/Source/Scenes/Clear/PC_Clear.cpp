#include "Scenes/Clear/PC_Clear.h"

#include <ActorManager.h>
#include <EnhancedInputComponent.h>
#include <NetworkManager.h>

#include <algorithm>
#include <iomanip>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>

#include "InputManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Clear/UI/WClearHUD.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "UIManager.h"
#include "World.h"

namespace {
enum : FNetworkRPCId { RPC_ClientReceiveWorldRanking = 1 };

nlohmann::json SerializeEntry(const FWorldRankingEntry& Entry) {
  return {
      {"rank", Entry.Rank},
      {"user_id", Entry.UserId},
      {"identity_key", Entry.IdentityKey},
      {"player_name", Entry.PlayerName},
      {"score", Entry.Score},
      {"is_self", Entry.bIsSelf}
  };
}

FWorldRankingEntry ParseEntry(const nlohmann::json& Value) {
  FWorldRankingEntry Entry;
  Entry.Rank = Value.value("rank", 0);
  Entry.UserId = Value.value("user_id", std::string{});
  Entry.IdentityKey = Value.value("identity_key", std::string{});
  Entry.PlayerName = Value.value("player_name", std::string{});
  Entry.Score = Value.value("score", 0.0f);
  Entry.bIsSelf = Value.value("is_self", false);
  return Entry;
}
}  // namespace

REGISTER_ACTOR(PC_Clear)

PC_Clear::PC_Clear() {
  SetUpdateableAnytime(true);
  RegisterRPC(
      RPC_ClientReceiveWorldRanking, ENetRPCType::Client, this, &PC_Clear::ClientReceiveWorldRanking
  );
}

void PC_Clear::BeginPlay() {
  APlayerController::BeginPlay();
  if (!bIsLocallyControlled) {
    return;
  }

  SetInputMode(EInputMode::UIOnly);
  SetupInputMappings();
  ClearHUD = GetWorld()->SpawnActor<WClearHUD>();
  UIManager::GetInstance()->AddWidget(ClearHUD);
  UIManager::GetInstance()->SetFocusedWidget(ClearHUD);
  if (!ClearHUD) {
    return;
  }

  const bool bIsHost = GetWorld()->IsServer();
  ClearHUD->SetHostMode(false);
  ClearHUD->SetWaitingForHost(!bIsHost);
  ClearHUD->SetWaitingForResults(true);
  ClearHUD->SetWorldRankingPending();
  ClearHUD->OnBackToLobby = [this]() {
    if (auto* Scene = dynamic_cast<AClearScene*>(GetWorld()->GetGameMode())) {
      Scene->RequestReturnToLobby();
    }
  };
}

void PC_Clear::OnUpdate(float DeltaTime) {
  APlayerController::OnUpdate(DeltaTime);
  if (bIsLocallyControlled) {
    if (MEnhancedInputComponent* Input = GetInputComponent()) {
      if (InputMapper* Mapper = GetInputMapper()) {
        Input->ProcessInputBindings(*Mapper, true, false);
      }
    }
  }

  if (!bIsLocallyControlled || !ClearHUD) {
    return;
  }
  RefreshMultiplayerResults();
  RefreshReturnCountdown();
}

bool PC_Clear::SendWorldRankingToOwner(
    bool bSuccess, const FWorldRankingBatchResult& Result, const std::string& IdentityKey
) {
  if (!bHasAuthority) {
    return false;
  }

  nlohmann::json Payload = {
      {"success", bSuccess}, {"identity_key", IdentityKey}, {"top", nlohmann::json::array()}
  };
  for (const auto& Entry : Result.Top) {
    Payload["top"].push_back(SerializeEntry(Entry));
  }

  Payload["self_rank"] = nullptr;
  Payload["around_self"] = nlohmann::json::array();
  const auto RankingIt = Result.RankingsByIdentity.find(IdentityKey);
  if (RankingIt != Result.RankingsByIdentity.end()) {
    if (RankingIt->second.SelfRank.has_value()) {
      Payload["self_rank"] = *RankingIt->second.SelfRank;
    }
    for (const auto& Entry : RankingIt->second.AroundSelf) {
      Payload["around_self"].push_back(SerializeEntry(Entry));
    }
  }

  return InvokeRPC(
      RPC_ClientReceiveWorldRanking,
      ENetRPCType::Client,
      ENetPacketReliability::Reliable,
      Payload.dump()
  );
}

void PC_Clear::ClientReceiveWorldRanking(std::string Payload) {
  if (!bIsLocallyControlled || !ClearHUD) {
    return;
  }

  try {
    const nlohmann::json Value = nlohmann::json::parse(Payload);
    if (!Value.value("success", false)) {
      ClearHUD->SetWorldRankingError();
      ClearHUD->SetHostMode(GetWorld()->IsServer());
      return;
    }

    const std::string IdentityKey = Value.value("identity_key", std::string{});
    FWorldRankingBatchResult Result;
    for (const auto& EntryValue : Value.value("top", nlohmann::json::array())) {
      Result.Top.push_back(ParseEntry(EntryValue));
    }

    FUserRankingData UserRanking;
    if (Value.contains("self_rank") && Value["self_rank"].is_number_integer()) {
      UserRanking.SelfRank = Value["self_rank"].get<int>();
    }
    for (const auto& EntryValue : Value.value("around_self", nlohmann::json::array())) {
      UserRanking.AroundSelf.push_back(ParseEntry(EntryValue));
    }
    Result.RankingsByIdentity[IdentityKey] = std::move(UserRanking);
    ClearHUD->SetWorldRanking(Result, IdentityKey);
    ClearHUD->SetHostMode(GetWorld()->IsServer());
  } catch (const nlohmann::json::exception&) {
    ClearHUD->SetWorldRankingError();
    ClearHUD->SetHostMode(GetWorld()->IsServer());
  }
}

std::vector<ALobbyPlayerState*> PC_Clear::GetResultStates() {
  std::map<FNetworkConnectionId, ALobbyPlayerState*> StateByConnectionId;
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return {};
  }
  for (const auto& ActorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    auto* State = dynamic_cast<ALobbyPlayerState*>(ActorPtr.get());
    if (!State || State->IsPendingDestroy()) {
      continue;
    }
    auto ExistingIt = StateByConnectionId.find(State->OwnerConnectionId);
    if (ExistingIt == StateByConnectionId.end() ||
        (State->bHasAuthority && !ExistingIt->second->bHasAuthority)) {
      StateByConnectionId[State->OwnerConnectionId] = State;
    }
  }

  std::vector<ALobbyPlayerState*> States;
  for (const auto& Pair : StateByConnectionId) {
    States.push_back(Pair.second);
  }
  std::sort(
      States.begin(), States.end(), [](const ALobbyPlayerState* A, const ALobbyPlayerState* B) {
        if (A->IsFinished() != B->IsFinished()) {
          return A->IsFinished() > B->IsFinished();
        }
        if (A->IsFinished() && A->GetFinishTime() != B->GetFinishTime()) {
          return A->GetFinishTime() < B->GetFinishTime();
        }
        return A->OwnerConnectionId < B->OwnerConnectionId;
      }
  );
  return States;
}

void PC_Clear::RefreshMultiplayerResults() {
  const FNetworkConnectionId LocalId = NetworkManager::GetInstance().GetLocalConnectionId();
  const auto States = GetResultStates();
  std::ostringstream Signature;
  Signature << std::fixed << std::setprecision(2);
  std::vector<FResultEntryViewData> Results;

  for (const auto* State : States) {
    const bool bLocalPlayer = (GetWorld()->IsServer() && State->OwnerConnectionId == 0) ||
                              State->OwnerConnectionId == LocalId || State->bIsLocallyControlled;
    FResultEntryViewData Data;
    Data.ConnectionId = State->OwnerConnectionId;
    Data.PlayerName = State->GetPlayerName();
    Data.bFinished = State->IsFinished();
    Data.FinishTime = State->GetFinishTime();
    Data.bLocalPlayer = bLocalPlayer;
    Results.push_back(Data);
    Signature << Data.ConnectionId << ':' << Data.PlayerName << ':' << Data.bFinished << ':'
              << Data.FinishTime << '|';
    if (bLocalPlayer && Data.bFinished) {
      ClearHUD->SetClearTime(Data.FinishTime);
    }
  }

  ClearHUD->SetWaitingForResults(Results.empty());
  const std::string NextSignature = Signature.str();
  if (NextSignature != LastResultSignature) {
    LastResultSignature = NextSignature;
    ClearHUD->SetMultiplayerResults(Results);
  }
}

void PC_Clear::RefreshReturnCountdown() {
  int Countdown = -1;
  for (const auto* State : GetResultStates()) {
    if (State && State->OwnerConnectionId == 0) {
      Countdown = State->GetStartCountdownSeconds();
      break;
    }
  }
  if (Countdown != LastDisplayedReturnCountdown) {
    LastDisplayedReturnCountdown = Countdown;
    ClearHUD->SetReturnCountdown(Countdown);
  }
}
