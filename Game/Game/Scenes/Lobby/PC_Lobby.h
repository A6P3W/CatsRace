#pragma once

#include <string>
#include <vector>

#include "NetworkTypes.h"
#include "PlayerController.h"

class ALobbyPlayerState;
class ALobbyScene;
class WLobbyHUD;
class WLeaveLobbyConfirmDialog;
class WMapSelectDialog;

class PC_Lobby : public APlayerController {
 public:
  DEFINE_ACTOR_CLASS(PC_Lobby)

  PC_Lobby();

  std::vector<ALobbyPlayerState*> GetPlayerStates();
  ALobbyPlayerState* FindLocalPlayerState();
  ALobbyPlayerState* FindHostPlayerState();
  ALobbyPlayerState* FindPlayerState(FNetworkConnectionId ConnectionId);
  int GetMaxPlayers() const;
  std::string GetSelectedLevelPath() const;
  void ApplyHostLobbyOptions();
  void SetMaxPlayers(int InMaxPlayers);
  void ShowMapSelectDialog();
  void ShowLeaveLobbyConfirmDialog();
  void StartGame();
  void LeaveLobby();
  bool IsStartCountdownActive() const;
  int GetStartCountdownSeconds() const;

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  ALobbyScene* GetLobbyScene() const;

  WLobbyHUD* m_LobbyHUD = nullptr;
  WLeaveLobbyConfirmDialog* m_LeaveLobbyConfirmDialog = nullptr;
  WMapSelectDialog* m_MapSelectDialog = nullptr;
};
