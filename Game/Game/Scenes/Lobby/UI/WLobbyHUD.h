#pragma once

#include <WidgetBase.h>

class PC_Lobby;

class WLobbyHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WLobbyHUD)

  void SetLobbyController(PC_Lobby* InLobbyController);
  void Draw() override;

 private:
  PC_Lobby* LobbyController = nullptr;
};