#pragma once
#include <GameInstance.h>
#include <NetworkTypes.h>

#include <array>
#include <random>
#include <string>
#include <vector>

#include "Core/MapData.h"

namespace PlayerNameDefaults {
inline std::string Generate() {
  static constexpr std::array<const char*, 10> Names = {
      "Nora", "Mike", "Kiji", "Tora", "Kuro", "Shiro", "Lion", "Jaguar", "=w=", "caaat"
  };
  static constexpr std::array<char, 10> SuffixChars = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
  };

  static std::random_device randomDevice;
  static std::mt19937 randomEngine(randomDevice());
  std::uniform_int_distribution<size_t> nameDistribution(0, Names.size() - 1);
  std::uniform_int_distribution<size_t> suffixDistribution(0, SuffixChars.size() - 1);

  std::string result = Names[nameDistribution(randomEngine)];
  for (int i = 0; i < 2; ++i) {
    result.push_back(SuffixChars[suffixDistribution(randomEngine)]);
  }
  return result;
}
}  // namespace PlayerNameDefaults

class GI_main : public GameInstance {
 public:
  struct FMultiplayerResult {
    FNetworkConnectionId ConnectionId = 0;
    std::string PlayerName = "Player";
    bool bFinished = false;
    float FinishTime = 0.0f;
  };

  float ClearTime = -1.0f;
  std::string map_id = "/";
  std::string user_id = "";
  std::string LastGhostData = "";

  std::string player_name = PlayerNameDefaults::Generate();
  std::string last_server_ip = "127.0.0.1";
  std::string last_level_path = AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
  std::vector<FMultiplayerResult> multiplayer_results;
};
