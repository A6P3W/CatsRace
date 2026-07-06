#pragma once

#include <WidgetBase.h>

#include <string>
#include <vector>

class MUIVerticalBoxComponent;
class MUIWidgetComponent;
class PC_Lobby;
class UIBoxButtonComponent;
class UITextComponent;
class UIToggleButtonComponent;

class WLobbyHUD : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WLobbyHUD)

  void SetLobbyController(PC_Lobby* InLobbyController);
  void Draw() override;

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  struct FPlayerRow {
    MUIWidgetComponent* RowRoot = nullptr;
    UITextComponent* StatusText = nullptr;
    UITextComponent* NameText = nullptr;
  };

  void UpdatePlayerList();
  void UpdateMapInfo();
  void UpdateLocalReadyState();
  void UpdateStartGameState();
  void RebuildNavigation();
  bool CanStartGame() const;

  PC_Lobby* LobbyController = nullptr;

  MUIVerticalBoxComponent* m_PlayerListBox = nullptr;
  MUIVerticalBoxComponent* m_ActionBox = nullptr;
  MUIVerticalBoxComponent* m_MapInfoBox = nullptr;

  UITextComponent* m_SelectedMapText = nullptr;
  UIToggleButtonComponent* m_ReadyToggle = nullptr;
  UIBoxButtonComponent* m_MapSelectButton = nullptr;
  UIBoxButtonComponent* m_StartGameButton = nullptr;
  UIBoxButtonComponent* m_LeaveButton = nullptr;

  std::vector<FPlayerRow> m_PlayerRows;

  int m_LastPlayerCount = 0;
  std::string m_LastMapPath = "";
  bool m_bLastHostMode = false;
  bool m_bLastCanStartGame = false;
};
