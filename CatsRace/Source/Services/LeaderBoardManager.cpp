#include "Services/LeaderBoardManager.h"

#include <HttpManager.h>
#include <Log.h>

#include <algorithm>
#include <nlohmann/json.hpp>
#include <utility>

namespace {
constexpr int SchemaVersion = 1;

FWorldRankingEntry ParseRankingEntry(const nlohmann::json& Value, bool bIncludeSelf) {
  FWorldRankingEntry Entry;
  Entry.Rank = Value.at("rank").get<int>();
  Entry.UserId = Value.at("user_id").get<std::string>();
  Entry.IdentityKey = Value.at("identity_key").get<std::string>();
  Entry.PlayerName = Value.at("player_name").get<std::string>();
  Entry.Score = Value.at("score").get<float>();
  Entry.bIsSelf = bIncludeSelf && Value.value("is_self", false);
  return Entry;
}

bool TryGetSuccessData(const HttpResponse& Response, nlohmann::json& OutData) {
  if (!Response.bSuccess) {
    return false;
  }
  const nlohmann::json Body = nlohmann::json::parse(Response.Body);
  if (Body.value("status", "") != "success" || !Body.contains("data") ||
      !Body["data"].is_object()) {
    return false;
  }
  OutData = Body["data"];
  return true;
}
}  // namespace

void LeaderBoardManager::OnUpdate(float DeltaTime) {
  std::vector<std::function<void()>> TimeoutCallbacks;
  for (const auto& Request : PendingRequests) {
    if (!Request || Request->bCompleted) {
      continue;
    }
    Request->ElapsedSeconds += DeltaTime;
    if (Request->ElapsedSeconds < RequestTimeoutSeconds) {
      continue;
    }
    Request->bCompleted = true;
    if (Request->TimeoutCallback) {
      TimeoutCallbacks.push_back(Request->TimeoutCallback);
    }
  }

  for (const auto& Callback : TimeoutCallbacks) {
    Callback();
  }
  std::erase_if(PendingRequests, [](const auto& Request) {
    return !Request || Request->bCompleted;
  });
}

std::shared_ptr<LeaderBoardManager::FPendingRequest> LeaderBoardManager::TrackRequest(
    std::function<void()> TimeoutCallback
) {
  auto Request = std::make_shared<FPendingRequest>();
  Request->TimeoutCallback = std::move(TimeoutCallback);
  PendingRequests.push_back(Request);
  return Request;
}

void LeaderBoardManager::FetchRaceGhosts(
    const std::string& MapId, int MapVersion, FetchRaceGhostsCallback Callback
) {
  if (!Callback) {
    return;
  }

  nlohmann::json RequestBody = {
      {"schema_version", SchemaVersion}, {"map_id", MapId}, {"map_version", MapVersion}
  };
  auto Request = TrackRequest([Callback]() { Callback(false, {}); });
  HttpManager::GetInstance().PostJson(
      this,
      GcfEndpoints::GetRaceGhosts,
      RequestBody.dump(),
      [Request, Callback](const HttpResponse& Response) {
        if (Request->bCompleted) {
          return;
        }
        Request->bCompleted = true;

        std::vector<FRaceGhostData> Ghosts;
        try {
          nlohmann::json Data;
          if (!TryGetSuccessData(Response, Data) || !Data.contains("ghosts") ||
              !Data["ghosts"].is_array()) {
            Callback(false, Ghosts);
            return;
          }
          for (const auto& Value : Data["ghosts"]) {
            FRaceGhostData Ghost;
            Ghost.UserId = Value.at("user_id").get<std::string>();
            Ghost.IdentityKey = Value.at("identity_key").get<std::string>();
            Ghost.PlayerName = Value.at("player_name").get<std::string>();
            Ghost.Score = Value.at("score").get<float>();
            Ghost.GhostSchemaVersion = Value.at("ghost_schema_version").get<int>();
            Ghost.GhostRecordedSeconds = Value.at("ghost_recorded_seconds").get<float>();
            Ghost.bIsGhostPartial = Value.at("is_ghost_partial").get<bool>();
            Ghost.GhostData = Value.at("ghost_data").get<std::string>();
            Ghosts.push_back(std::move(Ghost));
          }
          Callback(true, Ghosts);
        } catch (const nlohmann::json::exception& Error) {
          M_LOG(Error, "Failed to parse get-race-ghosts-v2 response: {}", Error.what());
          Callback(false, Ghosts);
        }
      }
  );
}

void LeaderBoardManager::PostRaceResultsBatch(
    const std::string& MapId,
    int MapVersion,
    const std::vector<FSingleRaceResultPostData>& Results,
    PostRaceResultsBatchCallback Callback
) {
  if (!Callback) {
    return;
  }

  nlohmann::json JsonResults = nlohmann::json::array();
  std::unordered_map<std::string, std::string> UserIdsByIdentity;
  for (const FSingleRaceResultPostData& Result : Results) {
    JsonResults.push_back({
        {"user_id", Result.UserId},
        {"id_type", Result.IdType},
        {"identity_key", Result.IdentityKey},
        {"player_name", Result.PlayerName},
        {"score", Result.Score},
        {"ghost_schema_version", Result.GhostSchemaVersion},
        {"ghost_recorded_seconds", Result.GhostRecordedSeconds},
        {"is_ghost_partial", Result.bIsGhostPartial},
        {"ghost_data", Result.GhostData},
    });
    UserIdsByIdentity[Result.IdentityKey] = Result.UserId;
  }
  const nlohmann::json RequestBody = {
      {"schema_version", SchemaVersion},
      {"map_id", MapId},
      {"map_version", MapVersion},
      {"results", JsonResults},
  };

  auto Request = TrackRequest([Callback]() { Callback(false, {}); });
  HttpManager::GetInstance().PostJson(
      this,
      GcfEndpoints::PostRaceResult,
      RequestBody.dump(),
      [Request, Callback, UserIdsByIdentity](const HttpResponse& Response) {
        if (Request->bCompleted) {
          return;
        }
        Request->bCompleted = true;

        std::vector<FPostRaceResultEntry> PostedResults;
        try {
          nlohmann::json Data;
          if (!TryGetSuccessData(Response, Data) || !Data.contains("results") ||
              !Data["results"].is_array()) {
            Callback(false, PostedResults);
            return;
          }
          for (const auto& Value : Data["results"]) {
            FPostRaceResultEntry Entry;
            Entry.IdentityKey = Value.at("identity_key").get<std::string>();
            if (const auto Found = UserIdsByIdentity.find(Entry.IdentityKey);
                Found != UserIdsByIdentity.end()) {
              Entry.UserId = Found->second;
            }
            Entry.bUpdatedBest = Value.at("updated_best").get<bool>();
            Entry.BestScore = Value.at("best_score").get<float>();
            PostedResults.push_back(std::move(Entry));
          }
          Callback(true, PostedResults);
        } catch (const nlohmann::json::exception& Error) {
          M_LOG(Error, "Failed to parse post-race-result-v2 response: {}", Error.what());
          Callback(false, PostedResults);
        }
      }
  );
}

void LeaderBoardManager::FetchWorldRankingBatch(
    const std::string& MapId,
    int MapVersion,
    const std::vector<std::string>& IdentityKeys,
    FetchWorldRankingBatchCallback Callback
) {
  if (!Callback) {
    return;
  }

  const nlohmann::json RequestBody = {
      {"schema_version", SchemaVersion},
      {"map_id", MapId},
      {"map_version", MapVersion},
      {"identity_keys", IdentityKeys},
  };
  auto Request = TrackRequest([Callback]() { Callback(false, {}); });
  HttpManager::GetInstance().PostJson(
      this,
      GcfEndpoints::GetWorldRanking,
      RequestBody.dump(),
      [Request, Callback](const HttpResponse& Response) {
        if (Request->bCompleted) {
          return;
        }
        Request->bCompleted = true;

        FWorldRankingBatchResult Result;
        try {
          nlohmann::json Data;
          if (!TryGetSuccessData(Response, Data) || !Data.contains("top") ||
              !Data["top"].is_array() || !Data.contains("rankings_by_identity") ||
              !Data["rankings_by_identity"].is_object()) {
            Callback(false, Result);
            return;
          }
          for (const auto& Value : Data["top"]) {
            Result.Top.push_back(ParseRankingEntry(Value, false));
          }
          for (const auto& [IdentityKey, Value] : Data["rankings_by_identity"].items()) {
            FUserRankingData UserRanking;
            if (Value.contains("self_rank") && !Value["self_rank"].is_null()) {
              UserRanking.SelfRank = Value["self_rank"].get<int>();
            }
            if (Value.contains("around_self") && Value["around_self"].is_array()) {
              for (const auto& AroundValue : Value["around_self"]) {
                UserRanking.AroundSelf.push_back(ParseRankingEntry(AroundValue, true));
              }
            }
            Result.RankingsByIdentity.emplace(IdentityKey, std::move(UserRanking));
          }
          Callback(true, Result);
        } catch (const nlohmann::json::exception& Error) {
          M_LOG(Error, "Failed to parse get-world-ranking-v2 response: {}", Error.what());
          Callback(false, Result);
        }
      }
  );
}
