#pragma once

#include <string>

#include "Actor.h"
#include "NetworkTypes.h"

class ALobbyPlayerState : public AActor {
 public:
  DEFINE_ACTOR_CLASS(ALobbyPlayerState)

  ALobbyPlayerState();
  ALobbyPlayerState(const FVector2D& Location, FRotator Rotation);

  void SetPlayerName(const std::string& Name);
  void SetReady(bool bInReady);
  void SetLobbyOptions(FNetworkSceneId InSelectedGameSceneId, int InMaxPlayers);
  void SetFinishResult(bool bInFinished, float InFinishTime);

  const std::string& GetPlayerName() const { return PlayerName; }
  bool IsReady() const { return bReady; }
  FNetworkSceneId GetSelectedGameSceneId() const { return SelectedGameSceneId; }
  int GetMaxPlayers() const { return MaxPlayers; }
  bool IsFinished() const { return bFinished; }
  float GetFinishTime() const { return FinishTime; }

 private:
  void InitializeRPCs();
  void ApplyPlayerName(const std::string& Name);
  void ApplyReady(bool bInReady);
  void ApplyLobbyOptions(FNetworkSceneId InSelectedGameSceneId, int InMaxPlayers);
  void ApplyFinishResult(bool bInFinished, float InFinishTime);

  std::string PlayerName = "Player";
  bool bReady = false;
  FNetworkSceneId SelectedGameSceneId = 10;
  int MaxPlayers = 4;
  bool bFinished = false;
  float FinishTime = 0.0f;
};
