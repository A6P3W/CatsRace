#include "Scenes/Menu/UI/MenuWidgets.h"

#include <Application.h>
#include <DxLib.h>
#include <UIBoxButton.h>
#include <UIInputTextComponent.h>
#include <UITextComponent.h>
#include <UIVerticalBoxComponent.h>

#include <algorithm>
#include <memory>
#include <string>

namespace {
constexpr float MenuButtonWidth = 360.0f;
constexpr float MenuButtonHeight = 64.0f;
constexpr float InputBoxWidth = 420.0f;
constexpr float PlayerNameInputHeight = 96.0f;
constexpr float LobbyNameInputHeight = 92.0f;
constexpr FColor InputNormalColor{102, 102, 102};
constexpr FColor InputHoveredColor{136, 136, 136};
constexpr FColor InputEditingColor{74, 111, 159};
constexpr FColor InputLabelColor{255, 255, 255};
constexpr int InputLabelFontSize = 20;
constexpr float InputLabelOffsetY = -28.0f;
constexpr float InputTextOffsetY = 7.0f;
constexpr float InputActionHintOffsetY = -18.0f;
constexpr const char* LobbyStateAttributeKey = "LOBBY_STATE";
constexpr const char* LobbyStateWaiting = "WAITING";
constexpr const char* LobbyStateRacing = "RACING";
constexpr FColor ButtonTextColor{255, 255, 255};
constexpr FColor DisabledButtonTextColor{136, 136, 136};
const FColor ButtonNormalColor = FColor{32, 38, 48};
const FColor ButtonHoveredColor = FColor{30, 115, 190};
const FColor ButtonPressedColor = FColor{15, 78, 140};
const FColor DisabledButtonColor = FColor{58, 58, 58};

UIBoxButtonComponent* AddButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width = MenuButtonWidth,
    float Height = MenuButtonHeight,
    const FColor& NormalColor = ButtonNormalColor,
    const FColor& HoveredColor = ButtonHoveredColor,
    const FColor& PressedColor = ButtonPressedColor,
    const FColor& TextColor = ButtonTextColor
) {
  auto* buttonPtr = NewObject<UIBoxButtonComponent>(Owner);
  buttonPtr->SetSize(Width, Height);
  buttonPtr->SetColors(NormalColor, HoveredColor, PressedColor);
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(buttonPtr);
  }
  buttonPtr->RegisterComponent();

  auto* text = NewObject<UITextComponent>(Owner);
  text->SetText(Label);
  text->SetColor(TextColor);
  text->SetFontSize(24);
  text->AttachToComponent(buttonPtr);
  text->SetAnchor(EUIAnchor::MiddleCenter);
  text->SetPivot({0.5f, 0.5f});
  text->SetAnchoredPosition({0.0f, 0.0f});
  text->RegisterComponent();

  return buttonPtr;
}

void AddInputLabel(AWidgetBase* Owner, UIInputTextComponent* Input, const std::string& Label) {
  auto* labelText = NewObject<UITextComponent>(Owner);
  labelText->SetText(Label);
  labelText->SetColor(InputLabelColor);
  labelText->SetFontSize(InputLabelFontSize);
  labelText->AttachToComponent(Input);
  labelText->SetAnchor(EUIAnchor::MiddleCenter);
  labelText->SetPivot({0.5f, 0.5f});
  labelText->SetAnchoredPosition({0.0f, InputLabelOffsetY});
  labelText->RegisterComponent();
}

std::string GetLobbyDisplayName(const FLobbyInfo& LobbyInfo) {
  std::string name = LobbyInfo.GetStringAttribute("LOBBYNAME", "");
  if (name.empty()) {
    name = LobbyInfo.GetStringAttribute("HOSTNAME", "Unknown Lobby");
  }
  return name;
}

bool IsLobbyRacing(const FLobbyInfo& LobbyInfo) {
  return LobbyInfo.GetStringAttribute(LobbyStateAttributeKey, LobbyStateWaiting) ==
         LobbyStateRacing;
}
}  // namespace

WMainMenuWidget::WMainMenuWidget() {
  ButtonList = NewObject<MUIVerticalBoxComponent>(this);
  ButtonList->SetAnchor(EUIAnchor::MiddleCenter);
  ButtonList->SetPivot({0.5f, 0.5f});
  ButtonList->SetAnchoredPosition({0.0f, 210.0f});
  ButtonList->SetSpacing(18.0f);
  ButtonList->RegisterComponent();

  CreateLobbyButton = AddButton(this, ButtonList, "OnlinePlay");

  UserNameInput = NewObject<UIInputTextComponent>(this);
  UserNameInput->SetSize(InputBoxWidth, PlayerNameInputHeight);
  UserNameInput->SetColors(InputNormalColor, InputHoveredColor, InputEditingColor);
  UserNameInput->SetTextColor(FColor{255, 255, 255});
  UserNameInput->SetHintColor(FColor{221, 221, 221});
  UserNameInput->SetTextOffsetY(InputTextOffsetY);
  UserNameInput->SetActionHintOffsetY(InputActionHintOffsetY);
  UserNameInput->SetPivot({0.5f, 0.5f});
  ButtonList->AddItem(UserNameInput);
  UserNameInput->RegisterComponent();
  AddInputLabel(this, UserNameInput, "プレイヤー名：");

  QuitGameButton = AddButton(this, ButtonList, "Quit Game");

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("");
  StatusText->SetColor(FColor{0, 0, 0});
  StatusText->SetFontSize(20);
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  StatusText->RegisterComponent();
}

void WMainMenuWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  ButtonList->BuildNavigation();
  SetFocusedButton(CreateLobbyButton);

  UserNameInput->SetOnTextChanged([this](const std::string& Text) {
    if (OnUserNameChanged) {
      OnUserNameChanged(Text);
    }
  });
  UserNameInput->SetOnTextCommitted([this](const std::string& Text) {
    if (OnUserNameChanged) {
      OnUserNameChanged(Text);
    }
  });
  CreateLobbyButton->SetOnPressed([this]() {
    if (OnCreateLobby) {
      OnCreateLobby();
    }
  });
  QuitGameButton->SetOnPressed([this]() {
    if (OnQuitGame) {
      OnQuitGame();
    }
  });
}

void WMainMenuWidget::SetInitialUserName(const std::string& Name) {
  if (UserNameInput) {
    UserNameInput->SetText(Name, false);
  }
}

void WMainMenuWidget::SetStatusText(const std::string& Text) {
  if (StatusText) {
    StatusText->SetText(Text);
  }
}

WCreateLobbyWidget::WCreateLobbyWidget() {
  auto* titleText = NewObject<UITextComponent>(this);
  titleText->SetText("Create Lobby");
  titleText->SetColor(FColor{255, 255, 255});
  titleText->SetFontSize(32);
  titleText->SetAnchor(EUIAnchor::MiddleLeft);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({430.0f, 20.0f});
  titleText->RegisterComponent();

  ControlList = NewObject<MUIVerticalBoxComponent>(this);
  ControlList->SetAnchor(EUIAnchor::MiddleLeft);
  ControlList->SetPivot({0.5f, 0.5f});
  ControlList->SetAnchoredPosition({430.0f, 130.0f});
  ControlList->SetSpacing(18.0f);
  ControlList->RegisterComponent();

  LobbyNameInput = NewObject<UIInputTextComponent>(this);
  LobbyNameInput->SetSize(InputBoxWidth, LobbyNameInputHeight);
  LobbyNameInput->SetColors(InputNormalColor, InputHoveredColor, InputEditingColor);
  LobbyNameInput->SetTextColor(FColor{255, 255, 255});
  LobbyNameInput->SetHintColor(FColor{221, 221, 221});
  LobbyNameInput->SetTextOffsetY(InputTextOffsetY);
  LobbyNameInput->SetActionHintOffsetY(InputActionHintOffsetY);
  LobbyNameInput->SetPivot({0.5f, 0.5f});
  LobbyNameInput->SetMaxLength(32);
  ControlList->AddItem(LobbyNameInput);
  LobbyNameInput->RegisterComponent();
  AddInputLabel(this, LobbyNameInput, "ロビー名：");

  CreateButton = AddButton(this, ControlList, "Create");
  BackButton = AddButton(this, ControlList, "Back");

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("");
  StatusText->SetColor(FColor{0, 0, 0});
  StatusText->SetFontSize(20);
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  StatusText->RegisterComponent();
}

void WCreateLobbyWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  ControlList->BuildNavigation();
  SetFocusedButton(LobbyNameInput);

  LobbyNameInput->SetOnTextChanged([this](const std::string& Text) {
    if (OnLobbyNameChanged) {
      OnLobbyNameChanged(Text);
    }
  });
  LobbyNameInput->SetOnTextCommitted([this](const std::string& Text) {
    if (OnLobbyNameChanged) {
      OnLobbyNameChanged(Text);
    }
  });
  CreateButton->SetOnPressed([this]() {
    if (OnCreate) {
      OnCreate();
    }
  });
  BackButton->SetOnPressed([this]() {
    if (OnBack) {
      OnBack();
    }
  });
}

void WCreateLobbyWidget::SetSearchNavigation(UIBoxButtonComponent* FirstLobbyButton) {
  if (!FirstLobbyButton) {
    return;
  }
  LobbyNameInput->Navigation.Right = FirstLobbyButton;
  CreateButton->Navigation.Right = FirstLobbyButton;
  BackButton->Navigation.Right = FirstLobbyButton;
  FirstLobbyButton->Navigation.Left = CreateButton;
}
void WCreateLobbyWidget::SetInitialLobbyName(const std::string& LobbyName) {
  if (LobbyNameInput) {
    LobbyNameInput->SetText(LobbyName, true);
  }
}

void WCreateLobbyWidget::SetStatusText(const std::string& Text) {
  if (StatusText) {
    StatusText->SetText(Text);
  }
}

WSearchLobbyWidget::WSearchLobbyWidget() {
  auto* titleText = NewObject<UITextComponent>(this);
  titleText->SetText("Lobbies");
  titleText->SetColor(FColor{255, 255, 255});
  titleText->SetFontSize(32);
  titleText->SetAnchor(EUIAnchor::MiddleRight);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({-430.0f, -180.0f});
  titleText->RegisterComponent();

  ResultList = NewObject<MUIVerticalBoxComponent>(this);
  ResultList->SetAnchor(EUIAnchor::MiddleRight);
  ResultList->SetPivot({0.5f, 0.0f});
  ResultList->SetAnchoredPosition({-430.0f, -130.0f});
  ResultList->SetSpacing(14.0f);
  ResultList->RegisterComponent();

  EmptyText = NewObject<UITextComponent>(this);
  EmptyText->SetText("No lobby search results.");
  EmptyText->SetColor(FColor{136, 149, 166});
  EmptyText->SetFontSize(20);
  EmptyText->SetPivot({0.5f, 0.5f});
  ResultList->AddItem(EmptyText);
  EmptyText->RegisterComponent();

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("");
  StatusText->SetColor(FColor{0, 0, 0});
  StatusText->SetFontSize(20);
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  StatusText->RegisterComponent();
}

void WSearchLobbyWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  RebuildNavigation();

}

void WSearchLobbyWidget::SetStatusText(const std::string& Text) {
  if (StatusText) {
    StatusText->SetText(Text);
  }
}

void WSearchLobbyWidget::SetLobbyResults(
    const std::vector<FLobbyInfo>& Results, int SelectedIndex
) {

  for (auto* button : LobbyButtons) {
    if (button) {
      ResultList->RemoveItem(button);
      button->DestroyComponent();
    }
  }
  for (auto* text : LobbyTexts) {
    if (text) {
      text->DestroyComponent();
    }
  }
  LobbyButtons.clear();
  JoinableLobbyButtons.clear();
  LobbyTexts.clear();

  if (EmptyText) {
    EmptyText->SetVisibility(Results.empty());
  }


  UIBoxButtonComponent* selectedButton = nullptr;
  auto addLobbyButton = [this, &Results, SelectedIndex, &selectedButton](int index) {
    const FLobbyInfo& lobbyInfo = Results[index];
    const bool isRacing = IsLobbyRacing(lobbyInfo);
    std::string label =
        GetLobbyDisplayName(lobbyInfo) + "  " + std::to_string(lobbyInfo.CurrentMembers) + "Cats";
    if (isRacing) {
      label += "  [レース中]";
    }

    UIBoxButtonComponent* button = nullptr;
    if (isRacing) {
      button = AddButton(
          this,
          ResultList,
          label,
          520.0f,
          56.0f,
          DisabledButtonColor,
          DisabledButtonColor,
          DisabledButtonColor,
          DisabledButtonTextColor
      );
    } else {
      button = AddButton(this, ResultList, label, 520.0f, 56.0f);
      button->SetOnPressed([this, index]() {
        if (OnLobbySelected) {
          OnLobbySelected(index);
        }
      });
    }

    LobbyButtons.push_back(button);
    if (!isRacing) {
      JoinableLobbyButtons.push_back(button);
    }
    if (!isRacing && index == SelectedIndex) {
      selectedButton = button;
    }
  };

  for (int index = 0; index < static_cast<int>(Results.size()); ++index) {
    if (!IsLobbyRacing(Results[index])) {
      addLobbyButton(index);
    }
  }


  for (int index = 0; index < static_cast<int>(Results.size()); ++index) {
    if (IsLobbyRacing(Results[index])) {
      addLobbyButton(index);
    }
  }

  RebuildNavigation();
}

UIBoxButtonComponent* WSearchLobbyWidget::GetFirstJoinableButton() const {
  return JoinableLobbyButtons.empty() ? nullptr : JoinableLobbyButtons.front();
}
void WSearchLobbyWidget::RebuildNavigation() {
  ResultList->BuildNavigation();

  for (auto* button : LobbyButtons) {
    if (button && std::find(JoinableLobbyButtons.begin(), JoinableLobbyButtons.end(), button) ==
                      JoinableLobbyButtons.end()) {
      button->Navigation.Up = nullptr;
      button->Navigation.Down = nullptr;
    }
  }
}