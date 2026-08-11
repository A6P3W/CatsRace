#pragma once

#include <array>
#include <string>

#include "Actor.h"

class World;

class AHostServerTravelActor : public AActor {
 public:
  DEFINE_ACTOR_CLASS(AHostServerTravelActor)

  AHostServerTravelActor();

  static void SpawnForListenServer(World& InWorld);

 void Draw() override;

 private:
  void SetLevelPathFromInput();
  void ExecuteServerTravel();
  bool NormalizeSelectedLevelPath(const std::string& InputPath, std::string& OutLevelPath) const;

  std::array<char, 1024> LevelPathInput{};
  std::string SelectedLevelPath;
  std::string StatusText;
};
