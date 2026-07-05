#include "Scenes/Menu/UI/MenuWidgets.h"

#include <Application.h>
#include <DxLib.h>
#include <UIBoxButton.h>
#include <UIInputTextComponent.h>
#include <UITextComponent.h>
#include <UIVerticalBoxComponent.h>

#include <memory>
#include <string>

namespace {
constexpr float MenuButtonWidth = 360.0f;
constexpr float MenuButtonHeight = 64.0f;

UIBoxButtonComponent* AddButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width = MenuButtonWidth,
    float Height = MenuButtonHeight
) {
  auto button = std::make_unique<UIBoxButtonComponent>(
      Width, Height, GetColor(32, 38, 48), GetColor(30, 115, 190), GetColor(15, 78, 140)
  );
  UIBoxButtonComponent* buttonPtr = button.get();
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(buttonPtr);
  }

  auto text = std::make_unique<UITextComponent>(Label, 0xFFFFFF, 24);
  text->SetParentComponent(buttonPtr);
  text->SetAnchor(EUIAnchor::MiddleCenter);
  text->SetPivot({0.5f, 0.5f});
  text->SetAnchoredPosition({0.0f, 0.0f});

  Owner->AddComponent(std::move(button));
  Owner->AddComponent(std::move(text));
  return buttonPtr;
}

std::string GetLobbyDisplayName(const FLobbyInfo& LobbyInfo) {
  std::string name = LobbyInfo.GetStringAttribute("LOBBYNAME", "");
  if (name.empty()) {
    name = LobbyInfo.GetStringAttribute("HOSTNAME", "Unknown Lobby");
  }
  return name;
}
}  // namespace

WMainMenuWidget::WMainMenuWidget() {
  auto buttonList = std::make_unique<MUIVerticalBoxComponent>();
  ButtonList = buttonList.get();
  ButtonList->SetAnchor(EUIAnchor::MiddleCenter);
  ButtonList->SetPivot({0.5f, 0.5f});
  ButtonList->SetAnchoredPosition({0.0f, 210.0f});
  ButtonList->SetSpacing(18.0f);
  AddComponent(std::move(buttonList));

  CreateLobbyButton = AddButton(this, ButtonList, "Create Lobby");
  SearchLobbyButton = AddButton(this, ButtonList, "Search Lobby");
  QuitGameButton = AddButton(this, ButtonList, "Quit Game");

  auto statusText = std::make_unique<UITextComponent>("", 0x000000, 20);
  StatusText = statusText.get();
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  AddComponent(std::move(statusText));
}

void WMainMenuWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  ButtonList->BuildNavigation();
  SetFocusedButton(CreateLobbyButton);

  CreateLobbyButton->OnPressed = [this]() {
    if (OnCreateLobby) {
      OnCreateLobby();
    }
  };
  SearchLobbyButton->OnPressed = [this]() {
    if (OnSearchLobby) {
      OnSearchLobby();
    }
  };
  QuitGameButton->OnPressed = [this]() {
    if (OnQuitGame) {
      OnQuitGame();
    }
  };
}

void WMainMenuWidget::SetStatusText(const std::string& Text) {
  if (StatusText) {
    StatusText->SetText(Text);
  }
}

WCreateLobbyWidget::WCreateLobbyWidget() {
  auto titleText = std::make_unique<UITextComponent>("Create Lobby", 0xFFFFFF, 32);
  titleText->SetAnchor(EUIAnchor::MiddleCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, 20.0f});
  AddComponent(std::move(titleText));

  auto controlList = std::make_unique<MUIVerticalBoxComponent>();
  ControlList = controlList.get();
  ControlList->SetAnchor(EUIAnchor::MiddleCenter);
  ControlList->SetPivot({0.5f, 0.5f});
  ControlList->SetAnchoredPosition({0.0f, 130.0f});
  ControlList->SetSpacing(18.0f);
  AddComponent(std::move(controlList));

  auto input = std::make_unique<UIInputTextComponent>(MenuButtonWidth, 56.0f, "Lobby Name");
  LobbyNameInput = input.get();
  LobbyNameInput->SetMaxLength(32);
  LobbyNameInput->SetPivot({0.5f, 0.5f});
  ControlList->AddItem(LobbyNameInput);
  AddComponent(std::move(input));

  CreateButton = AddButton(this, ControlList, "Create");
  BackButton = AddButton(this, ControlList, "Back");

  auto statusText = std::make_unique<UITextComponent>("", 0x000000, 20);
  StatusText = statusText.get();
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  AddComponent(std::move(statusText));
}

void WCreateLobbyWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  ControlList->BuildNavigation();
  SetFocusedButton(LobbyNameInput);

  LobbyNameInput->OnTextChanged = [this](const std::string& Text) {
    if (OnLobbyNameChanged) {
      OnLobbyNameChanged(Text);
    }
  };
  LobbyNameInput->OnTextCommitted = [this](const std::string& Text) {
    if (OnLobbyNameChanged) {
      OnLobbyNameChanged(Text);
    }
  };
  CreateButton->OnPressed = [this]() {
    if (OnCreate) {
      OnCreate();
    }
  };
  BackButton->OnPressed = [this]() {
    if (OnBack) {
      OnBack();
    }
  };
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
  auto titleText = std::make_unique<UITextComponent>("Search Lobby", 0xFFFFFF, 32);
  titleText->SetAnchor(EUIAnchor::TopCenter);
  titleText->SetPivot({0.5f, 0.5f});
  titleText->SetAnchoredPosition({0.0f, 150.0f});
  AddComponent(std::move(titleText));

  auto resultList = std::make_unique<MUIVerticalBoxComponent>();
  ResultList = resultList.get();
  ResultList->SetAnchor(EUIAnchor::TopCenter);
  ResultList->SetPivot({0.5f, 0.0f});
  ResultList->SetAnchoredPosition({0.0f, 230.0f});
  ResultList->SetSpacing(14.0f);
  AddComponent(std::move(resultList));

  RefreshButton = AddButton(this, ResultList, "Refresh");

  auto emptyText = std::make_unique<UITextComponent>("No lobby search results.", 0x8895A6, 20);
  EmptyText = emptyText.get();
  EmptyText->SetPivot({0.5f, 0.5f});
  ResultList->AddItem(EmptyText);
  AddComponent(std::move(emptyText));

  BackButton = AddButton(this, ResultList, "Back");

  auto statusText = std::make_unique<UITextComponent>("", 0x000000, 20);
  StatusText = statusText.get();
  StatusText->SetAnchor(EUIAnchor::BottomCenter);
  StatusText->SetPivot({0.5f, 0.5f});
  StatusText->SetAnchoredPosition({0.0f, -56.0f});
  AddComponent(std::move(statusText));
}

void WSearchLobbyWidget::BeginPlay() {
  AWidgetBase::BeginPlay();

  RebuildNavigation();
  SetFocusedButton(RefreshButton);

  RefreshButton->OnPressed = [this]() {
    if (OnRefresh) {
      OnRefresh();
    }
  };
  BackButton->OnPressed = [this]() {
    if (OnBack) {
      OnBack();
    }
  };
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
  LobbyTexts.clear();

  if (EmptyText) {
    EmptyText->SetVisibility(Results.empty());
  }

  for (int index = 0; index < static_cast<int>(Results.size()); ++index) {
    const FLobbyInfo& lobbyInfo = Results[index];
    const std::string label =
        GetLobbyDisplayName(lobbyInfo) + "  " + std::to_string(lobbyInfo.CurrentMembers) + "/" +
        std::to_string(lobbyInfo.MaxMembers);
    UIBoxButtonComponent* button = AddButton(this, ResultList, label, 520.0f, 56.0f);
    button->OnPressed = [this, index]() {
      if (OnLobbySelected) {
        OnLobbySelected(index);
      }
    };
    if (index == SelectedIndex) {
      SetFocusedButton(button);
    }
    LobbyButtons.push_back(button);
  }

  RebuildNavigation();
}

void WSearchLobbyWidget::RebuildNavigation() {
  ResultList->BuildNavigation();
  if (!LobbyButtons.empty()) {
    RefreshButton->Navigation.Down = LobbyButtons.front();
    LobbyButtons.front()->Navigation.Up = RefreshButton;
    LobbyButtons.back()->Navigation.Down = BackButton;
    BackButton->Navigation.Up = LobbyButtons.back();
  } else {
    RefreshButton->Navigation.Down = BackButton;
    BackButton->Navigation.Up = RefreshButton;
  }
}
