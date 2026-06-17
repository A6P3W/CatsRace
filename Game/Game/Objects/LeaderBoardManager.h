#pragma once
#include "Actor.h"
#include <string>
#include <vector>
#include <functional>

struct FLeaderBoardEntry {
    std::string user_id;
    float score;
};

using FetchLeaderBoardCallBack = std::function<void(const std::vector<FLeaderBoardEntry>&)>;
class LeaderBoardManager : public AActor
{
public:
    DEFINE_ACTOR_CLASS(LeaderBoardManager);

    void FetchLeaderBoard(const std::string map_id,FetchLeaderBoardCallBack callback);
    void PostScore(const std::string);

private:
    std::string FetchLeaderBoardUrl = "https://get-race-leaderboards-bcpidymjjq-uw.a.run.app";
    std::string PostScoreUrl = "https://post-race-time-bcpidymjjq-uw.a.run.app";
};