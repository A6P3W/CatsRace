#pragma once

#include <functional>
#include <string>
#include <vector>

#include "EOSTypes.h"
#include "WidgetBase.h"

class MUIVerticalBoxComponent;
class UIBoxButtonComponent;
class UIInputTextComponent;
class UITextComponent;

class WMainMenuWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WMainMenuWidget)

  WMainMenuWidget();
  void SetStatusText(const std::string& Text);

  std::function<void()> OnCreateLobby;
  std::function<void()> OnSearchLobby;
  std::function<void()> OnQuitGame;

 protected:
  void BeginPlay() override;

 private:
  MUIVerticalBoxComponent* ButtonList = nullptr;
  UIBoxButtonComponent* CreateLobbyButton = nullptr;
  UIBoxButtonComponent* SearchLobbyButton = nullptr;
  UIBoxButtonComponent* QuitGameButton = nullptr;
  UITextComponent* StatusText = nullptr;
};

class WCreateLobbyWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WCreateLobbyWidget)

  WCreateLobbyWidget();
  void SetInitialLobbyName(const std::string& LobbyName);
  void SetStatusText(const std::string& Text);

  std::function<void(const std::string&)> OnLobbyNameChanged;
  std::function<void()> OnCreate;
  std::function<void()> OnBack;

 protected:
  void BeginPlay() override;

 private:
  MUIVerticalBoxComponent* ControlList = nullptr;
  UIInputTextComponent* LobbyNameInput = nullptr;
  UIBoxButtonComponent* CreateButton = nullptr;
  UIBoxButtonComponent* BackButton = nullptr;
  UITextComponent* StatusText = nullptr;
};

class WSearchLobbyWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WSearchLobbyWidget)

  WSearchLobbyWidget();
  void SetStatusText(const std::string& Text);
  void SetLobbyResults(const std::vector<FLobbyInfo>& Results, int SelectedIndex);

  std::function<void()> OnRefresh;
  std::function<void()> OnBack;
  std::function<void(int)> OnLobbySelected;

 protected:
  void BeginPlay() override;

 private:
  void RebuildNavigation();

  MUIVerticalBoxComponent* ResultList = nullptr;
  UIBoxButtonComponent* RefreshButton = nullptr;
  UIBoxButtonComponent* BackButton = nullptr;
  UITextComponent* EmptyText = nullptr;
  UITextComponent* StatusText = nullptr;
  std::vector<UIBoxButtonComponent*> LobbyButtons;
  std::vector<UITextComponent*> LobbyTexts;
};