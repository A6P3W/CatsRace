#include "ClearScene.h"

#include <ActorManager.h>
#include <Pawn.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include "Actors/HostServerTravelActor.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "SceneManager.h"
#include "Scenes/Clear/PC_Clear.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "World.h"

REGISTER_GAME_MODE(AClearScene)

AClearScene::AClearScene() { SetDefaultPlayerControllerClass("PC_Clear"); }

void AClearScene::BeginPlay() {
  AGameModeBase::BeginPlay();
  AHostServerTravelActor::SpawnForListenServer(*GetWorld());

  if (GetWorld()->IsServer()) {
    SpawnResultStatesFromGameInstance();
    BeginCloudResultFlow();
  }
}

void AClearScene::SpawnResultStatesFromGameInstance() {
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance) {
    return;
  }

  for (const auto& Result : GameInstance->multiplayer_results) {
    auto* State = GetWorld()->SpawnActor<ALobbyPlayerState>();
    if (!State) {
      continue;
    }

    State->OwnerConnectionId = Result.ConnectionId;
    State->bReplicates = true;
    State->bHasAuthority = true;
    State->bIsLocallyControlled = Result.ConnectionId == 0;
    State->SetPlayerName(Result.PlayerName);
    State->SetFinishResult(Result.bFinished, Result.FinishTime);
  }
}

void AClearScene::BeginCloudResultFlow() {
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance) {
    FWorldRankingBatchResult EmptyResult;
    DistributeWorldRanking(false, EmptyResult);
    return;
  }

  RankingManager = GetWorld()->SpawnActor<LeaderBoardManager>();
  if (!RankingManager) {
    FWorldRankingBatchResult EmptyResult;
    DistributeWorldRanking(false, EmptyResult);
    return;
  }

  std::vector<FSingleRaceResultPostData> Results;
  for (const auto& RaceResult : GameInstance->multiplayer_results) {
    if (!RaceResult.bFinished || RaceResult.FinishTime <= 0.0f || RaceResult.UserId.empty() ||
        RaceResult.IdentityKey.empty() || RaceResult.GhostRecordedSeconds <= 0.0f ||
        RaceResult.GhostData.empty()) {
      continue;
    }

    FSingleRaceResultPostData PostData;
    PostData.UserId = RaceResult.UserId;
    PostData.IdType = RaceResult.IdType;
    PostData.IdentityKey = RaceResult.IdentityKey;
    PostData.PlayerName = RaceResult.PlayerName;
    PostData.Score = RaceResult.FinishTime;
    PostData.GhostSchemaVersion = RaceResult.GhostSchemaVersion;
    PostData.GhostRecordedSeconds = RaceResult.GhostRecordedSeconds;
    PostData.bIsGhostPartial = RaceResult.bIsGhostPartial;
    PostData.GhostData = RaceResult.GhostData;
    Results.push_back(std::move(PostData));
  }

  if (Results.empty()) {
    FetchWorldRanking();
    return;
  }

  ResultFlowState = EResultFlowState::PostingBatch;
  RankingManager->PostRaceResultsBatch(
      GameInstance->RaceMapId,
      GameInstance->RaceMapVersion,
      Results,
      [this](bool, const std::vector<FPostRaceResultEntry>&) { FetchWorldRanking(); }
  );
}

std::vector<std::string> AClearScene::GetParticipantIdentityKeys() const {
  std::vector<std::string> IdentityKeys;
  std::unordered_set<std::string> Seen;
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance) {
    return IdentityKeys;
  }

  for (const auto& Result : GameInstance->multiplayer_results) {
    if (!Result.IdentityKey.empty() && Seen.insert(Result.IdentityKey).second) {
      IdentityKeys.push_back(Result.IdentityKey);
    }
  }
  return IdentityKeys;
}

void AClearScene::FetchWorldRanking() {
  auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!RankingManager || !GameInstance) {
    FWorldRankingBatchResult EmptyResult;
    DistributeWorldRanking(false, EmptyResult);
    return;
  }

  ResultFlowState = EResultFlowState::FetchingRanking;
  RankingManager->FetchWorldRankingBatch(
      GameInstance->RaceMapId,
      GameInstance->RaceMapVersion,
      GetParticipantIdentityKeys(),
      [this](bool bSuccess, const FWorldRankingBatchResult& Result) {
        DistributeWorldRanking(bSuccess, Result);
      }
  );
}

void AClearScene::DistributeWorldRanking(bool bSuccess, const FWorldRankingBatchResult& Result) {
  ResultFlowState = EResultFlowState::Ready;
  bWorldRankingSucceeded = bSuccess;
  CachedWorldRanking = Result;
  DeliverWorldRankingToPendingControllers();
}

void AClearScene::DeliverWorldRankingToPendingControllers() {
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }

  std::unordered_map<FNetworkConnectionId, std::string> IdentityByConnection;
  if (auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    for (const auto& RaceResult : GameInstance->multiplayer_results) {
      IdentityByConnection[RaceResult.ConnectionId] = RaceResult.IdentityKey;
    }
  }

  for (const auto& ActorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    auto* Controller = dynamic_cast<PC_Clear*>(ActorPtr.get());
    if (!Controller || !Controller->bHasAuthority ||
        RankingDeliveredConnections.contains(Controller->OwnerConnectionId)) {
      continue;
    }
    const auto IdentityIt = IdentityByConnection.find(Controller->OwnerConnectionId);
    const std::string IdentityKey =
        IdentityIt == IdentityByConnection.end() ? std::string{} : IdentityIt->second;
    if (Controller->SendWorldRankingToOwner(
            bWorldRankingSucceeded, CachedWorldRanking, IdentityKey
        )) {
      RankingDeliveredConnections.insert(Controller->OwnerConnectionId);
    }
  }
}

void AClearScene::RequestReturnToLobby() {
  if (!GetWorld() || !GetWorld()->IsServer() || bReturnToLobbyRequested) {
    return;
  }
  if (ResultFlowState != EResultFlowState::Ready) {
    return;
  }
  bReturnToLobbyRequested = true;
  ReturnToLobbyRemaining = ReturnToLobbyDelaySeconds;
  PublishReturnCountdown(static_cast<int>(ReturnToLobbyDelaySeconds));
}

void AClearScene::PublishReturnCountdown(int Seconds) {
  LastPublishedReturnCountdown = Seconds;
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }
  for (const auto& ActorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    if (auto* State = dynamic_cast<ALobbyPlayerState*>(ActorPtr.get())) {
      if (State->bHasAuthority) {
        State->SetStartCountdownSeconds(Seconds);
      }
    }
  }
}

void AClearScene::OnUpdate(float DeltaTime) {
  if (GetWorld()->IsServer() && GetWorld()->GetActorManager()) {
    for (const auto& ActorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
      if (auto* Player = dynamic_cast<APlayer*>(ActorPtr.get())) {
        Player->Destroy();
      }
    }
  }

  if (GetWorld()->IsServer() && ResultFlowState == EResultFlowState::Ready) {
    DeliverWorldRankingToPendingControllers();
  }

  if (!GetWorld()->IsServer() || !bReturnToLobbyRequested) {
    return;
  }

  ReturnToLobbyRemaining = (std::max)(0.0f, ReturnToLobbyRemaining - DeltaTime);
  const int Countdown = static_cast<int>(std::ceil(ReturnToLobbyRemaining));
  if (Countdown != LastPublishedReturnCountdown) {
    PublishReturnCountdown(Countdown);
  }
  if (ReturnToLobbyRemaining <= 0.0f) {
    GetWorld()->ServerTravel(GameSceneIds::Lobby);
  }
}
