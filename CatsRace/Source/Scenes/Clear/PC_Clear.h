#pragma once

#include <string>
#include <vector>

#include "PlayerController.h"
#include "Services/LeaderBoardManager.h"

class WClearHUD;
class ALobbyPlayerState;

class PC_Clear : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Clear)

  PC_Clear();
  void OnUpdate(float DeltaTime) override;
  bool SendWorldRankingToOwner(
      bool bSuccess, const FWorldRankingBatchResult& Result, const std::string& IdentityKey
  );

 protected:
  void BeginPlay() override;

 private:
  void ClientReceiveWorldRanking(std::string Payload);
  void RefreshMultiplayerResults();
  void RefreshReturnCountdown();
  std::vector<ALobbyPlayerState*> GetResultStates();

  WClearHUD* ClearHUD = nullptr;
  std::string LastResultSignature;
  int LastDisplayedReturnCountdown = -1;
};
