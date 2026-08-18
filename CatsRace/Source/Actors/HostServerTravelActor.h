#pragma once

#include <array>
#include <string>
#include <vector>

#include "Actor.h"
#include "NetworkTypes.h"

class World;
class APlayer;
class ALobbyPlayerState;

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

  struct FLobbyPlayerEntry {
    FNetworkConnectionId ConnectionId = 0;
    ALobbyPlayerState* PlayerState = nullptr;
    std::array<char, 33> NameInput{};
    std::string LastObservedName;
  };

  void InitializePlayerEntries();
  void RefreshLobbyPlayerEntries();
  void DrawLobbyPlayerEntries();
  void SetLobbyPlayerNameInput(FLobbyPlayerEntry& Entry, const std::string& Name);
  bool HasDuplicateBoothName(
      FNetworkConnectionId TargetConnectionId, const std::string& Name
  ) const;
  void SetLevelPathFromInput();
  void ExecuteServerTravel();
  bool NormalizeSelectedLevelPath(const std::string& InputPath, std::string& OutLevelPath) const;

  std::array<char, 1024> LevelPathInput{};
  std::string SelectedLevelPath;
  std::string StatusText;
  std::string LobbyPlayerStatusText;
  std::vector<FPlayerSpeedEntry> PlayerEntries;
  std::vector<FLobbyPlayerEntry> LobbyPlayerEntries;
  bool bPlayerEntriesInitialized = false;
};
