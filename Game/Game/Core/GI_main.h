#pragma once
#include <GameInstance.h>
#include <NetworkTypes.h>
#include <string>
#include <vector>

class GI_main : public GameInstance
{
public:
	struct FMultiplayerResult
	{
		FNetworkConnectionId ConnectionId = 0;
		std::string PlayerName = "Player";
		bool bFinished = false;
		float FinishTime = 0.0f;
	};

	float ClearTime = 1000;
	std::string map_id = "/";
	std::string user_id = "";
	std::string LastGhostData = "";

	std::string player_name = "Player";
	std::string last_server_ip = "127.0.0.1";
	FNetworkSceneId last_game_scene_id = 10;
	std::vector<FMultiplayerResult> multiplayer_results;
};