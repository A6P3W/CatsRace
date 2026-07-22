#pragma once

#include <string>
#include <vector>

struct FMapInfo {
  std::string DisplayName;
  std::string LevelPath;
  std::string LobbyPreviewPath;
};

inline const std::vector<FMapInfo> AvailableMaps = {
    {"Stage1", "Resources-EOS/Stage1/Stage1.BLevel", "Resources-EOS/Stage1/Stage1_Preview.png"},
    {"Stage2", "Resources-EOS/Stage2/Stage2.BLevel", "Resources-EOS/Stage2/Stage2_Preview.png"},
    {"Stage3", "Resources-EOS/Stage3/Stage3.BLevel", "Resources-EOS/Stage3/Stage3_Preview.png"},
    {"debug", "Resources/GameDebugScene.BLevel", "Resources-EOS/Stage3/Stage3_Preview.png"}
};

inline const FMapInfo* FindMapInfo(const std::string& LevelPath) {
  for (const FMapInfo& MapInfo : AvailableMaps) {
    if (MapInfo.LevelPath == LevelPath) {
      return &MapInfo;
    }
  }
  return nullptr;
}
