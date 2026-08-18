#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameModeBase.h"
#include "Services/LeaderBoardManager.h"

class ALobbyPlayerState;

class AClearScene : public AGameModeBase {
 public:
  DEFINE_ACTOR_CLASS(AClearScene)

  AClearScene();
  void OnUpdate(float DeltaTime) override;
  void RequestReturnToLobby();

 protected:
  void BeginPlay() override;

 private:
  enum class EResultFlowState { Idle, PostingBatch, FetchingRanking, Ready };

  void SpawnResultStatesFromGameInstance();
  void BeginCloudResultFlow();
  void FetchWorldRanking();
  void DistributeWorldRanking(bool bSuccess, const FWorldRankingBatchResult& Result);
  void DeliverWorldRankingToPendingControllers();
  void PublishReturnCountdown(int Seconds);
  std::vector<std::string> GetParticipantIdentityKeys() const;

  static constexpr float ReturnToLobbyDelaySeconds = 3.0f;
  EResultFlowState ResultFlowState = EResultFlowState::Idle;
  LeaderBoardManager* RankingManager = nullptr;
  FWorldRankingBatchResult CachedWorldRanking;
  std::unordered_set<FNetworkConnectionId> RankingDeliveredConnections;
  float ReturnToLobbyRemaining = 0.0f;
  int LastPublishedReturnCountdown = -1;
  bool bReturnToLobbyRequested = false;
  bool bWorldRankingSucceeded = false;
};
