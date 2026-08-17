#pragma once

#include <array>
#include <string>
#include <vector>

#include "Actor.h"

class World;
class APlayer;

class AHostServerTravelActor : public AActor {
 public:
  DEFINE_ACTOR_CLASS(AHostServerTravelActor)

  AHostServerTravelActor();

  static void SpawnForListenServer(World& InWorld);

  void Draw() override;

 private:
  struct FPlayerSpeedEntry {
    APlayer* Player = nullptr;
  };

  void InitializePlayerEntries();
  void SetLevelPathFromInput();
  void ExecuteServerTravel();
  bool NormalizeSelectedLevelPath(const std::string& InputPath, std::string& OutLevelPath) const;

  std::array<char, 1024> LevelPathInput{};
  std::string SelectedLevelPath;
  std::string StatusText;
  std::vector<FPlayerSpeedEntry> PlayerEntries;
  bool bPlayerEntriesInitialized = false;
};
