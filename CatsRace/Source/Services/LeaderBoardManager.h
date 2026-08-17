#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Actor.h"

#define GCF_USE_LOCAL_SERVER 1

namespace GcfEndpoints {
#if GCF_USE_LOCAL_SERVER
inline const std::string BaseUrl = "http://localhost:8080";
inline const std::string GetRaceGhosts = BaseUrl + "/get-race-ghosts-v2";
inline const std::string PostRaceResult = BaseUrl + "/post-race-result-v2";
inline const std::string GetWorldRanking = BaseUrl + "/get-world-ranking-v2";
#else
inline const std::string GetRaceGhosts = "https://get-race-ghosts-v2-xxxx-uw.a.run.app";
inline const std::string PostRaceResult = "https://post-race-result-v2-xxxx-uw.a.run.app";
inline const std::string GetWorldRanking = "https://get-world-ranking-v2-xxxx-uw.a.run.app";
#endif
}  // namespace GcfEndpoints

struct FWorldRankingEntry {
  int Rank = 0;
  std::string UserId;
  std::string IdentityKey;
  std::string PlayerName;
  float Score = 0.0f;
  bool bIsSelf = false;
};

struct FRaceGhostData {
  std::string UserId;
  std::string IdentityKey;
  std::string PlayerName;
  float Score = 0.0f;
  int GhostSchemaVersion = 1;
  float GhostRecordedSeconds = 0.0f;
  bool bIsGhostPartial = false;
  std::string GhostData;
};

struct FUserRankingData {
  std::optional<int> SelfRank;
  std::vector<FWorldRankingEntry> AroundSelf;
};

struct FWorldRankingBatchResult {
  std::vector<FWorldRankingEntry> Top;
  std::unordered_map<std::string, FUserRankingData> RankingsByIdentity;
};

struct FSingleRaceResultPostData {
  std::string UserId;
  std::string IdType;
  std::string IdentityKey;
  std::string PlayerName;
  float Score = 0.0f;
  int GhostSchemaVersion = 1;
  float GhostRecordedSeconds = 0.0f;
  bool bIsGhostPartial = false;
  std::string GhostData;
};

struct FPostRaceResultEntry {
  std::string UserId;
  std::string IdentityKey;
  bool bUpdatedBest = false;
  float BestScore = 0.0f;
};

using FetchRaceGhostsCallback =
    std::function<void(bool bSuccess, const std::vector<FRaceGhostData>& Ghosts)>;
using PostRaceResultsBatchCallback =
    std::function<void(bool bSuccess, const std::vector<FPostRaceResultEntry>& Results)>;
using FetchWorldRankingBatchCallback =
    std::function<void(bool bSuccess, const FWorldRankingBatchResult& Result)>;

class LeaderBoardManager : public AActor {
 public:
  DEFINE_ACTOR_CLASS(LeaderBoardManager)

  void OnUpdate(float DeltaTime) override;
  void FetchRaceGhosts(const std::string& MapId, int MapVersion, FetchRaceGhostsCallback Callback);
  void PostRaceResultsBatch(
      const std::string& MapId,
      int MapVersion,
      const std::vector<FSingleRaceResultPostData>& Results,
      PostRaceResultsBatchCallback Callback
  );
  void FetchWorldRankingBatch(
      const std::string& MapId,
      int MapVersion,
      const std::vector<std::string>& IdentityKeys,
      FetchWorldRankingBatchCallback Callback
  );

 private:
  struct FPendingRequest {
    float ElapsedSeconds = 0.0f;
    bool bCompleted = false;
    std::function<void()> TimeoutCallback;
  };

  std::shared_ptr<FPendingRequest> TrackRequest(std::function<void()> TimeoutCallback);

  static constexpr float RequestTimeoutSeconds = 5.0f;
  std::vector<std::shared_ptr<FPendingRequest>> PendingRequests;
};
