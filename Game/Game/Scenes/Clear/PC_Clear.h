#pragma once
#include <string>
#include <vector>

#include "PlayerController.h"

class WClearHUD;
class WNameSelectDialog;
class WNameInputDialog;
class WOverwriteConfirmDialog;
class WPostGameDialog;
class ALobbyPlayerState;

class PC_Clear : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Clear)

  PC_Clear();

  void Draw() override;

 protected:
  void BeginPlay() override;

 private:
  void ShowNameFlow();
  void ShowNameInputDialog();
  void CheckDuplicateAndPost(const std::string& name);
  void ExecutePostScore(const std::string& name);
  void FetchAndDisplay();
  void ShowPostGameDialog();
  void SpawnResultStatesFromGameInstance();

  std::vector<ALobbyPlayerState*> GetResultStates();

  WClearHUD* m_ClearHUD = nullptr;
  std::vector<std::string> m_FetchedUserIds;
  WNameSelectDialog* m_NameSelectDialog = nullptr;
  WNameInputDialog* m_NameInputDialog = nullptr;
  WOverwriteConfirmDialog* m_OverwriteDialog = nullptr;
  WPostGameDialog* m_PostGameDialog = nullptr;
};
