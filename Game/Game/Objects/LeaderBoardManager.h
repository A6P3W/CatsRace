#pragma once
#include "Actor.h"
#include <string>
#include <vector>
#include <functional>

struct FLeaderBoardEntry {
	std::string user_id = "";
	float score = 1000;
	std::string delta_timestamp = "";
};

using FetchLeaderBoardCallBack = std::function<void(bool bSuccess,const std::vector<FLeaderBoardEntry>&)>;
class LeaderBoardManager : public AActor
{
public:
	DEFINE_ACTOR_CLASS(LeaderBoardManager);

	void FetchLeaderBoard(const std::string map_id, FetchLeaderBoardCallBack callback);
	void PostScore(const std::string map_id, const std::string user_id, std::function<void(bool)> callback = nullptr);

private:
	std::string FetchLeaderBoardUrl = "https://get-race-leaderboards-bcpidymjjq-uw.a.run.app";
	std::string PostScoreUrl = "https://post-race-time-bcpidymjjq-uw.a.run.app";
};