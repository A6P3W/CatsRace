#pragma once

#include <string>
#include <vector>

struct FMapInfo {
  std::string DisplayName;
  std::string LevelPath;
  std::string LobbyPreviewPath;
  std::string MapId;
  int MapVersion = 1;
};

inline const std::vector<FMapInfo> AvailableMaps = {
    {"Stage1", "/Game/images/Stage1.BLevel", "/Game/images/Stage1_Preview.png", "stage1", 3},
    {"Stage2", "/Game/images/Stage2.BLevel", "/Game/images/Stage2_Preview.png", "stage2", 3},
    {"Stage3", "Resources-EOS/Stage3.BLevel", "/Game/images/Stage3_Preview.png", "stage3", 3},
};

inline const FMapInfo* FindMapInfo(const std::string& LevelPath) {
  for (const FMapInfo& MapInfo : AvailableMaps) {
    if (MapInfo.LevelPath == LevelPath) {
      return &MapInfo;
    }
  }
  return nullptr;
}
