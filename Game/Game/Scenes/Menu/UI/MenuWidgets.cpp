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
constexpr int InputNormalColor = 0x666666;
constexpr int InputHoveredColor = 0x888888;
constexpr int InputEditingColor = 0x4A6F9F;
constexpr int InputLabelColor = 0xFFFFFF;
constexpr int InputLabelFontSize = 20;
constexpr float InputLabelOffsetY = -28.0f;
constexpr float InputTextOffsetY = 7.0f;
constexpr float InputActionHintOffsetY = -18.0f;
constexpr const char* LobbyStateAttributeKey = "LOBBY_STATE";
constexpr const char* LobbyStateWaiting = "WAITING";
constexpr const char* LobbyStateRacing = "RACING";
constexpr int ButtonTextColor = 0xFFFFFF;
constexpr int DisabledButtonTextColor = 0x888888;
const int ButtonNormalColor = GetColor(32, 38, 48);
const int ButtonHoveredColor = GetColor(30, 115, 190);
const int ButtonPressedColor = GetColor(15, 78, 140);
const int DisabledButtonColor = GetColor(58, 58, 58);

UIBoxButtonComponent* AddButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width = MenuButtonWidth,
    float Height = MenuButtonHeight,
    int NormalColor = ButtonNormalColor,
    int HoveredColor = ButtonHoveredColor,
    int PressedColor = ButtonPressedColor,
    int TextColor = ButtonTextColor
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

  CreateLobbyButton = AddButton(this, ButtonList, "Create Lobby");
  SearchLobbyButton = AddButton(this, ButtonList, "Search Lobby");

  UserNameInput = NewObject<UIInputTextComponent>(this);
  UserNameInput->SetSize(InputBoxWidth, PlayerNameInputHeight);
  UserNameInput->SetColors(InputNormalColor, InputHoveredColor, InputEditingColor);
  UserNameInput->SetTextColor(0xFFFFFF);
  UserNameInput->SetHintColor(0xDDDDDD);
  UserNameInput->SetTextOffsetY(InputTextOffsetY);
  UserNameInput->SetActionHintOffsetY(InputActionHintOffsetY);
  UserNameInput->SetPivot({0.5f, 0.5f});
  ButtonList->AddItem(UserNameInput);
  UserNameInput->RegisterComponent();
  AddInputLabel(this, UserNameInput, "プレイヤー名：");

  QuitGameButton = AddButton(this, ButtonList, "Quit Game");

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("");
  StatusText->SetColor(0x000000);
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
  SearchLobbyButton->SetOnPressed([this]() {
    if (OnSearchLobby) {
      OnSearchLobby();
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
  titleText->SetColor(0xFFFFFF);
  titleText->SetFontSize(32);
  titleText->SetAnchor(EUIAnchor::MiddleCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, 20.0f});
  titleText->RegisterComponent();

  ControlList = NewObject<MUIVerticalBoxComponent>(this);
  ControlList->SetAnchor(EUIAnchor::MiddleCenter);
  ControlList->SetPivot({0.5f, 0.5f});
  ControlList->SetAnchoredPosition({0.0f, 130.0f});
  ControlList->SetSpacing(18.0f);
  ControlList->RegisterComponent();

  LobbyNameInput = NewObject<UIInputTextComponent>(this);
  LobbyNameInput->SetSize(InputBoxWidth, LobbyNameInputHeight);
  LobbyNameInput->SetColors(InputNormalColor, InputHoveredColor, InputEditingColor);
  LobbyNameInput->SetTextColor(0xFFFFFF);
  LobbyNameInput->SetHintColor(0xDDDDDD);
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
  StatusText->SetColor(0x000000);
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
  titleText->SetText("Search Lobby");
  titleText->SetColor(0xFFFFFF);
  titleText->SetFontSize(32);
  titleText->SetAnchor(EUIAnchor::TopCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, 150.0f});
  titleText->RegisterComponent();

  ResultList = NewObject<MUIVerticalBoxComponent>(this);
  ResultList->SetAnchor(EUIAnchor::TopCenter);
  ResultList->SetPivot({0.5f, 0.0f});
  ResultList->SetAnchoredPosition({0.0f, 230.0f});
  ResultList->SetSpacing(14.0f);
  ResultList->RegisterComponent();

  RefreshButton = AddButton(this, ResultList, "Refresh");

  EmptyText = NewObject<UITextComponent>(this);
  EmptyText->SetText("No lobby search results.");
  EmptyText->SetColor(0x8895A6);
  EmptyText->SetFontSize(20);
  EmptyText->SetPivot({0.5f, 0.5f});
  ResultList->AddItem(EmptyText);
  EmptyText->RegisterComponent();

  BackButton = AddButton(this, ResultList, "Back");

  StatusText = NewObject<UITextComponent>(this);
  StatusText->SetText("");
  StatusText->SetColor(0x000000);
  StatusText->SetFontSize(20);
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  StatusText->RegisterComponent();
}

void WSearchLobbyWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  RebuildNavigation();
  SetFocusedButton(RefreshButton);

  RefreshButton->SetOnPressed([this]() {
    if (OnRefresh) {
      OnRefresh();
    }
  });
  BackButton->SetOnPressed([this]() {
    if (OnBack) {
      OnBack();
    }
  });
}

void WSearchLobbyWidget::SetStatusText(const std::string& Text) {
  if (StatusText) {
    StatusText->SetText(Text);
  }
}

void WSearchLobbyWidget::SetLobbyResults(
    const std::vector<FLobbyInfo>& Results,
    int SelectedIndex
) {
  SetFocusedButton(RefreshButton);

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

  ResultList->RemoveItem(BackButton);

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

  ResultList->AddItem(BackButton);

  for (int index = 0; index < static_cast<int>(Results.size()); ++index) {
    if (IsLobbyRacing(Results[index])) {
      addLobbyButton(index);
    }
  }

  RebuildNavigation();

  if (selectedButton) {
    SetFocusedButton(selectedButton);
  } else {
    SetFocusedButton(RefreshButton);
  }
}

void WSearchLobbyWidget::RebuildNavigation() {
  ResultList->BuildNavigation();

  RefreshButton->Navigation.Up = nullptr;
  BackButton->Navigation.Down = nullptr;

  if (!JoinableLobbyButtons.empty()) {
    RefreshButton->Navigation.Down = JoinableLobbyButtons.front();
    JoinableLobbyButtons.front()->Navigation.Up = RefreshButton;
    JoinableLobbyButtons.back()->Navigation.Down = BackButton;
    BackButton->Navigation.Up = JoinableLobbyButtons.back();
  } else {
    RefreshButton->Navigation.Down = BackButton;
    BackButton->Navigation.Up = RefreshButton;
  }

  BackButton->Navigation.Down = nullptr;
  for (auto* button : LobbyButtons) {
    if (button &&
        std::find(JoinableLobbyButtons.begin(), JoinableLobbyButtons.end(), button) ==
            JoinableLobbyButtons.end()) {
      button->Navigation.Up = nullptr;
      button->Navigation.Down = nullptr;
    }
  }
}
