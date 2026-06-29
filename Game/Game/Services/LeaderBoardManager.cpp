#include "LeaderBoardManager.h"

#include <DxLib.h>
#include <HttpManager.h>
#include <Log.h>

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core/GI_main.h"
#include "SceneManager.h"

namespace {
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> GhostDataCache;

std::string ConvertUtf8ToSjis(const std::string& utf8Str) {
  if (utf8Str.empty()) return "";

  size_t bufferSize = utf8Str.size() * 2 + 1;
  std::vector<char> buffer(bufferSize, 0);

  int result = ConvertStringCharCodeFormat(
      DX_CHARCODEFORMAT_UTF8, utf8Str.c_str(), DX_CHARCODEFORMAT_SHIFTJIS, buffer.data()
  );

  if (result == -1) {
    return utf8Str;
  }

  return std::string(buffer.data());
}
}  // namespace

void LeaderBoardManager::FetchLeaderBoard(std::string map_id, FetchLeaderBoardCallBack callback) {
  nlohmann::json j;
  j["map_id"] = map_id;
  HttpManager::GetInstance().PostJson(
      this, FetchLeaderBoardUrl, j.dump(), [this, callback](const HttpResponse& res) {
        std::vector<FLeaderBoardEntry> LB;
        if (res.bSuccess) {
          try {
            M_LOG("{}", res.Body);
            auto body = nlohmann::json::parse(res.Body);
            nlohmann::json data = body["data"];

            for (const nlohmann::json& rank : data) {
              FLeaderBoardEntry entry;
              std::string utf8_user_id = rank["user_id"].get<std::string>();
              std::string utf8_delta = rank.value("delta_timestamp", "");

              entry.user_id = ConvertUtf8ToSjis(utf8_user_id);
              entry.score = rank["score"].get<float>();
              entry.delta_timestamp = ConvertUtf8ToSjis(utf8_delta);
              LB.push_back(entry);
            }
            callback(true, LB);
          } catch (const nlohmann::json::exception& e) {
          }
        } else {
          callback(false, LB);
        }
      }
  );
}

void LeaderBoardManager::PostScore(
    const std::string map_id, const std::string user_id, std::function<void(bool)> callback
) {
  auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  float score = gi ? gi->ClearTime : 0.0f;

  nlohmann::json j;
  j["map_id"] = map_id;
  j["user_id"] = user_id;
  j["score"] = score;
  j["ghost_data"] = gi ? gi->LastGhostData : "";

  HttpManager::GetInstance().PostJson(
      this, PostScoreUrl, j.dump(), [this, callback](const HttpResponse& res) {
        if (res.bSuccess) {
          M_LOG("Score posted successfully: {}", res.Body);
          if (callback) {
            callback(true);
          }
        } else {
          M_LOG("Failed to post score: {}", res.ErrorMessage);
          if (callback) {
            callback(false);
          }
        }
      }
  );
}
void LeaderBoardManager::FetchGhostData(
    const std::string map_id, const std::vector<std::string>& ids, FetchGhostDataCallBack callback
) {
  if (!callback) {
    return;
  }

  std::unordered_map<std::string, std::string> ghostDataById;
  std::vector<std::string> missingIds;

  const auto mapCacheIt = GhostDataCache.find(map_id);
  for (const auto& id : ids) {
    if (mapCacheIt != GhostDataCache.end()) {
      const auto ghostDataIt = mapCacheIt->second.find(id);
      if (ghostDataIt != mapCacheIt->second.end()) {
        ghostDataById[id] = ghostDataIt->second;
        continue;
      }
    }

    missingIds.push_back(id);
  }

  if (missingIds.empty()) {
    callback(true, ghostDataById);
    return;
  }

  nlohmann::json j;
  j["map_id"] = map_id;
  j["ids"] = missingIds;

  HttpManager::GetInstance().PostJson(
      this,
      FetchGhostDataUrl,
      j.dump(),
      [this, map_id, callback, ghostDataById](const HttpResponse& res) mutable {
        if (!res.bSuccess) {
          callback(!ghostDataById.empty(), ghostDataById);
          return;
        }

        try {
          auto body = nlohmann::json::parse(res.Body);
          if (body.value("status", "") == "success" && body.contains("data") &&
              body["data"].is_object()) {
            for (const auto& ghostData : body["data"].items()) {
              if (ghostData.value().is_string()) {
                const std::string id = ghostData.key();
                const std::string data = ghostData.value().get<std::string>();
                GhostDataCache[map_id][id] = data;
                ghostDataById[id] = data;
              }
            }

            callback(true, ghostDataById);
            return;
          }
        } catch (const nlohmann::json::exception& e) {
          M_LOG("Failed to parse ghost data response: {}", e.what());
        }

        callback(false, ghostDataById);
      }
  );
}
