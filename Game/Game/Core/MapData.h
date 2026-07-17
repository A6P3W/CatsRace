#pragma once

#include <string>
#include <vector>

struct FMapInfo {
  std::string DisplayName;
  std::string LevelPath;
};

inline const std::vector<FMapInfo> AvailableMaps = {
    {"Stage1", "Resources/GameScene01.BLevel"},
    {"Stage2", "Resources/GameScene02.BLevel"},
    {"debug", "Resources/GameDebugScene.BLevel"}
};
