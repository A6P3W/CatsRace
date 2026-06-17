#include "LeaderBoardManager.h"
#include <nlohmann/json.hpp>
#include <HttpManager.h>
#include <Log.h>
#include <string>
void LeaderBoardManager::FetchLeaderBoard(std::string map_id, FetchLeaderBoardCallBack callback)
{
	nlohmann::json j;
	j["map_id"] = map_id;
	HttpManager::GetInstance().PostJson(this, FetchLeaderBoardUrl, j.dump(), [this, callback](const HttpResponse& res) {
		if (res.bSuccess) {
			try {
				M_LOG("{}",res.Body);
				auto body = nlohmann::json::parse(res.Body);
				nlohmann::json data = body["data"];
				std::vector<FLeaderBoardEntry>LB;
				for (const nlohmann::json& rank : data) {
					FLeaderBoardEntry entry;
					entry.user_id = rank["user_id"].get<std::string>();
					entry.score = rank["score"].get<float>();
					LB.push_back(entry);
				}
				callback(LB);
			}
			catch (const nlohmann::json::exception& e) {

			}
		}
		});
}
