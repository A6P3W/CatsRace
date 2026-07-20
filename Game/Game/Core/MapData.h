#pragma once

#include <string>
#include <vector>

struct FMapInfo {
  std::string DisplayName;
  std::string LevelPath;
  std::string LobbyPreviewPath;
};

inline const std::vector<FMapInfo> AvailableMaps = {
    {"Stage1", "Resources/GameScene01.BLevel", "Resources/images/MapPreview/1.png"},
    {"Stage2", "Resources/GameScene02.BLevel", "Resources/images/MapPreview/2.png"},
    {"debug", "Resources/GameDebugScene.BLevel", "Resources/images/MapPreview/otamesiStage.png"}
};

inline const FMapInfo* FindMapInfo(const std::string& LevelPath) {
  for (const FMapInfo& MapInfo : AvailableMaps) {
    if (MapInfo.LevelPath == LevelPath) {
      return &MapInfo;
    }
  }
  return nullptr;
}
