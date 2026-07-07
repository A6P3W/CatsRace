#pragma once

#include <string>
#include <vector>

struct FMapInfo {
  std::string DisplayName;
  std::string LevelPath;
};

inline const std::vector<FMapInfo> AvailableMaps = {
    {"Cat Circuit", "Resources/GameScene01.BLevel"},
    {"Debug", "Resources/GameDebugScene.BLevel"},
    {"Stage2", "Resources/GameScene02.BLevel"},
    {"test", "Resources/debugtest.BLevel"},
};
