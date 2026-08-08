#pragma once

#include <string>
#include <vector>

struct FGhostFrame {
  int X = 0;
  int Y = 0;
  int Rot = 0;
};

struct FGhostData {
  std::string UserName;
  std::vector<FGhostFrame> Frames;
};

namespace GhostDataSerializer {
std::string Serialize(const std::vector<FGhostFrame>& Frames);
std::vector<FGhostFrame> Deserialize(const std::string& SerializedData);
}  // namespace GhostDataSerializer
