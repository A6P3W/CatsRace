#pragma once

class FPlayerRaceProgress {
 public:
  int GetCurrentLap() const { return CurrentLap; }
  int& GetReplicatedLap() { return CurrentLap; }
  int GetLastPassedCheckpoint() const { return LastPassedCheckpoint; }
  void SetLastPassedCheckpoint(int Index) { LastPassedCheckpoint = Index; }
  bool CanCompleteLap(int TotalCheckpoints) const;
  int CompleteLap();
  void SetReplicatedLap(int NewLap, int TotalLaps);

 private:
  int CurrentLap = 0;
  int LastPassedCheckpoint = -1;
};
