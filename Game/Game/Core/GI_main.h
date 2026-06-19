#pragma once
#include <GameInstance.h>
#include <string>
class GI_main : public GameInstance
{
public:
	float ClearTime=1000;
	std::string map_id = "/";
	std::string user_id = "";
};