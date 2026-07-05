#pragma once

#include <string>

#include "Actor.h"

class ALobbyPlayerState : public AActor {
 public:
  DEFINE_ACTOR_CLASS(ALobbyPlayerState)

  ALobbyPlayerState();
  ALobbyPlayerState(const FVector2D& Location, FRotator Rotation);

  void SetPlayerName(const std::string& Name);
  void SetReady(bool bInReady);
  void SetLobbyOptions(const std::string& InSelectedLevelPath, int InMaxPlayers);
  void SetFinishResult(bool bInFinished, float InFinishTime);

  const std::string& GetPlayerName() const { return PlayerName; }
  bool IsReady() const { return bReady; }
  const std::string& GetSelectedLevelPath() const { return SelectedLevelPath; }
  int GetMaxPlayers() const { return MaxPlayers; }
  bool IsFinished() const { return bFinished; }
  float GetFinishTime() const { return FinishTime; }

 private:
  void InitializeRPCs();
  void ApplyPlayerName(const std::string& Name);
  void ApplyReady(bool bInReady);
  void ApplyLobbyOptions(std::string InSelectedLevelPath, int InMaxPlayers);
  void ApplyFinishResult(bool bInFinished, float InFinishTime);

  std::string PlayerName = "Player";
  bool bReady = false;
  std::string SelectedLevelPath;
  int MaxPlayers = 4;
  bool bFinished = false;
  float FinishTime = 0.0f;
};
