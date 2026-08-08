#pragma once
#include <string>
#include <vector>

#include "PlayerController.h"

class WClearHUD;
class ALobbyPlayerState;

class PC_Clear : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Clear)

  PC_Clear();

  void OnUpdate(float DeltaTime) override;

 protected:
  void BeginPlay() override;

 private:
  void ExecutePostScore(const std::string& name);
  void FetchAndDisplay();
  void RefreshMultiplayerResults();
  void SubmitLocalResultToServerIfNeeded();
  void Server_SubmitLocalResult(float FinishTime);

  std::vector<ALobbyPlayerState*> GetResultStates();

  WClearHUD* m_ClearHUD = nullptr;
  std::string m_LastResultSignature;
  bool m_bSubmittedLocalResult = false;
  float m_ResultSubmitRetryCooldown = 0.0f;
  float DisplayReturnCountdownRemaining = 5.0f;
  int LastDisplayedReturnCountdown = -1;
  std::vector<std::string> m_FetchedUserIds;
};
