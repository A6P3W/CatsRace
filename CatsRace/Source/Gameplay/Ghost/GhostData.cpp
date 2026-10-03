#include "Ghost/GhostData.h"

#include <sstream>

namespace {
bool TryParseFrame(const std::string& Text, FGhostFrame& OutFrame) {
  std::stringstream ss(Text);
  std::string x;
  std::string y;
  std::string rot;
  std::string extra;

  if (!std::getline(ss, x, ',')) return false;
  if (!std::getline(ss, y, ',')) return false;
  if (!std::getline(ss, rot, ',')) return false;
  if (std::getline(ss, extra, ',')) return false;

  try {
    OutFrame.X = std::stoi(x);
    OutFrame.Y = std::stoi(y);
    OutFrame.Rot = std::stoi(rot);
  } catch (...) {
    return false;
  }

  return true;
}
}  // namespace

std::string GhostDataSerializer::Serialize(const std::vector<FGhostFrame>& Frames) {
  std::ostringstream oss;

  for (size_t i = 0; i < Frames.size(); ++i) {
    if (i > 0) {
      oss << '|';
    }

    const auto& frame = Frames[i];
    oss << frame.X << ',' << frame.Y << ',' << frame.Rot;
  }

  return oss.str();
}

std::vector<FGhostFrame> GhostDataSerializer::Deserialize(const std::string& SerializedData) {
  std::vector<FGhostFrame> frames;
  std::stringstream ss(SerializedData);
  std::string frameText;

  while (std::getline(ss, frameText, '|')) {
    if (frameText.empty()) {
      continue;
    }

    FGhostFrame frame;
    if (TryParseFrame(frameText, frame)) {
      frames.push_back(frame);
    }
  }

  return frames;
}
