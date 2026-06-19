#include "LeaderBoardManager.h"
#include <nlohmann/json.hpp>
#include <HttpManager.h>
#include <Log.h>
#include <string>
#include "GI_main.h"
#include "SceneManager.h"
#include <vector>
#include <DxLib.h>

namespace {
std::string ConvertUtf8ToSjis(const std::string& utf8Str)
{
	if (utf8Str.empty()) return "";

	size_t bufferSize = utf8Str.size() * 2 + 1;
	std::vector<char> buffer(bufferSize, 0);

	int result = ConvertStringCharCodeFormat(
		DX_CHARCODEFORMAT_UTF8,
		utf8Str.c_str(),
		DX_CHARCODEFORMAT_SHIFTJIS,
		buffer.data()
	);

	if (result == -1) {
		return utf8Str;
	}

	return std::string(buffer.data());
}
}

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
					std::string utf8_user_id = rank["user_id"].get<std::string>();
					std::string utf8_delta = rank.value("delta_timestamp", "");

					entry.user_id = ConvertUtf8ToSjis(utf8_user_id);
					entry.score = rank["score"].get<float>();
					entry.delta_timestamp = ConvertUtf8ToSjis(utf8_delta);
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

