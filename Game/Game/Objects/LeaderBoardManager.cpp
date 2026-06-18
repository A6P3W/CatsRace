#include "LeaderBoardManager.h"
#include <nlohmann/json.hpp>
#include <HttpManager.h>
#include <Log.h>
#include <string>
#include "GI_main.h"
#include "SceneManager.h"

void LeaderBoardManager::FetchLeaderBoard(std::string map_id, FetchLeaderBoardCallBack callback)
{
	nlohmann::json j;
	j["map_id"] = map_id;
	HttpManager::GetInstance().PostJson(this, FetchLeaderBoardUrl, j.dump(), [this, callback](const HttpResponse& res) {
		std::vector<FLeaderBoardEntry>LB;
		if (res.bSuccess) {
			try {
				M_LOG("{}", res.Body);
				auto body = nlohmann::json::parse(res.Body);
				nlohmann::json data = body["data"];

				for (const nlohmann::json& rank : data) {
					FLeaderBoardEntry entry;
					entry.user_id = rank["user_id"].get<std::string>();
					entry.score = rank["score"].get<float>();
					LB.push_back(entry);
				}
				callback(true, LB);
			}
			catch (const nlohmann::json::exception& e) {
				
			}
		}
		else {
			callback(false, LB);
		}
		});
}

void LeaderBoardManager::PostScore(const std::string map_id, const std::string user_id, std::function<void(bool)> callback)
{
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	float score = gi ? gi->ClearTime : 0.0f;

	nlohmann::json j;
	j["map_id"] = map_id;
	j["user_id"] = user_id;
	j["score"] = score;

	HttpManager::GetInstance().PostJson(this, PostScoreUrl, j.dump(), [this, callback](const HttpResponse& res) {
		if (res.bSuccess) {
			M_LOG("Score posted successfully: {}", res.Body);
			if (callback) {
				callback(true);
			}
		}
		else {
			M_LOG("Failed to post score: {}", res.ErrorMessage);
			if (callback) {
				callback(false);
			}
		}
		});
}

