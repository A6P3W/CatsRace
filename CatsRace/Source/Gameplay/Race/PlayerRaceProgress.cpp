#include "PlayerRaceProgress.h"

#include <algorithm>

bool FPlayerRaceProgress::CanCompleteLap(int TotalCheckpoints) const {
  return TotalCheckpoints <= 0 || LastPassedCheckpoint >= TotalCheckpoints - 1;
}

int FPlayerRaceProgress::CompleteLap() {
  LastPassedCheckpoint = -1;
  return ++CurrentLap;
}

void FPlayerRaceProgress::SetReplicatedLap(int NewLap, int TotalLaps) {
  CurrentLap = std::min(NewLap, TotalLaps);
}
