#pragma once

#include <functional>
#include <string>
#include <vector>

#include "EOSTypes.h"
#include "WidgetBase.h"

class MUIVerticalBoxComponent;
class MSpriteComponent;
class UIBoxButtonComponent;
class UIInputTextComponent;
class UITextComponent;

class WMainMenuWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WMainMenuWidget)

  WMainMenuWidget();
  void SetStatusText(const std::string& Text);
  void SetInitialUserName(const std::string& Name);

  std::function<void()> OnCreateLobby;
  std::function<void()> OnSearchLobby;
  std::function<void()> OnQuitGame;
  std::function<void(const std::string&)> OnUserNameChanged;

 protected:
  void BeginPlay() override;

 private:
  MUIVerticalBoxComponent* ButtonList = nullptr;
  UIBoxButtonComponent* CreateLobbyButton = nullptr;
  UIBoxButtonComponent* SearchLobbyButton = nullptr;
  UIInputTextComponent* UserNameInput = nullptr;
  UIBoxButtonComponent* QuitGameButton = nullptr;
  UITextComponent* StatusText = nullptr;
};

class WCreateLobbyWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WCreateLobbyWidget)

  WCreateLobbyWidget();
  void SetInitialLobbyName(const std::string& LobbyName);
  void SetStatusText(const std::string& Text);
  void SetSearchNavigation(UIBoxButtonComponent* FirstLobbyButton);
  void FocusCreateButton();
  void Navigate(const FInputActionValue& Value) override;

  std::function<void(const std::string&)> OnLobbyNameChanged;
  std::function<void()> OnCreate;
  std::function<void()> OnBack;
  std::function<void()> OnFocusSearchResults;

 protected:
  void BeginPlay() override;

 private:
  MUIVerticalBoxComponent* ControlList = nullptr;
  UIInputTextComponent* LobbyNameInput = nullptr;
  UIBoxButtonComponent* CreateButton = nullptr;
  UIBoxButtonComponent* BackButton = nullptr;
  UITextComponent* StatusText = nullptr;
  UIBoxButtonComponent* FirstSearchButton = nullptr;
};

class WSearchLobbyWidget : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WSearchLobbyWidget)

  WSearchLobbyWidget();
  void SetStatusText(const std::string& Text);
  void SetLobbyResults(const std::vector<FLobbyInfo>& Results, int SelectedIndex);
  UIBoxButtonComponent* GetFirstJoinableButton() const;
  bool FocusFirstJoinableButton();
  void Navigate(const FInputActionValue& Value) override;

  std::function<void()> OnBack;
  std::function<void()> OnFocusCreate;
  std::function<void(int)> OnLobbySelected;

 protected:
  void BeginPlay() override;

 private:
  void RebuildNavigation();

  MUIVerticalBoxComponent* ResultList = nullptr;
  UITextComponent* EmptyText = nullptr;
  UITextComponent* StatusText = nullptr;
  std::vector<UIBoxButtonComponent*> LobbyButtons;
  std::vector<UIBoxButtonComponent*> JoinableLobbyButtons;
  std::vector<UITextComponent*> LobbyTexts;
};

class WJoinLobbyDialog : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WJoinLobbyDialog)
  WJoinLobbyDialog();
  void SetLobbyName(const std::string& Name);
  std::function<void()> OnJoin;
  std::function<void()> OnBack;

 protected:
  void BeginPlay() override;

 private:
  MSpriteComponent* Panel = nullptr;
  UITextComponent* LobbyNameText = nullptr;
  UIBoxButtonComponent* JoinButton = nullptr;
  UIBoxButtonComponent* BackButton = nullptr;
};