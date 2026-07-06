#include "Scenes/Lobby/UI/WLobbyHUD.h"

#include <DxLib.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>
#include <UIToggleButtonComponent.h>
#include <UIVerticalBoxComponent.h>
#include <UIWidgetComponent.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Core/GI_main.h"
#include "Core/MapData.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "World.h"

namespace {
constexpr float PlayerRowWidth = 360.0f;
constexpr float PlayerRowHeight = 30.0f;
constexpr float ActionButtonWidth = 220.0f;
constexpr float ActionButtonHeight = 54.0f;
constexpr float MapButtonWidth = 220.0f;
constexpr float MapButtonHeight = 48.0f;
constexpr float LeaveButtonWidth = 240.0f;
constexpr float LeaveButtonHeight = 52.0f;

constexpr int ButtonNormalColor = 0x202630;
constexpr int ButtonHoveredColor = 0x1E73BE;
constexpr int ButtonPressedColor = 0x0F4E8C;
constexpr int ButtonDisabledColor = 0x555A62;
constexpr int LeaveNormalColor = 0x403232;
constexpr int LeaveHoveredColor = 0x964646;
constexpr int LeavePressedColor = 0x6E2D2D;
constexpr int ReadyOnColor = 0x1D8752;
constexpr int ReadyOnHoveredColor = 0x28A968;
constexpr int ReadyOnPressedColor = 0x176A41;
constexpr int ReadyOffColor = 0x555A62;
constexpr int ReadyOffHoveredColor = 0x6B7280;
constexpr int ReadyOffPressedColor = 0x474C55;
constexpr int ReadyTextColor = 0x5BE388;
constexpr int NotReadyTextColor = 0xFF8A80;

std::string FindMapDisplayName(const std::string& LevelPath) {
  const auto it = std::find_if(AvailableMaps.begin(), AvailableMaps.end(), [&LevelPath](const FMapInfo& Map) {
    return Map.LevelPath == LevelPath;
  });
  return it == AvailableMaps.end() ? LevelPath : it->DisplayName;
}

UITextComponent* AddText(
    AWidgetBase* Owner,
    MUIWidgetComponent* Parent,
    const std::string& Text,
    int Color,
    int FontSize,
    const FVector2D& WidgetSize,
    EUIAnchor Anchor,
    const FVector2D& Pivot,
    const FVector2D& Position
) {
  auto text = std::make_unique<UITextComponent>(Text, Color, FontSize);
  UITextComponent* textPtr = text.get();
  textPtr->SetParentComponent(Parent);
  textPtr->SetWidgetSize(WidgetSize);
  textPtr->SetAnchor(Anchor);
  textPtr->SetPivot(Pivot);
  textPtr->SetAnchoredPosition(Position);
  Owner->AddComponent(std::move(text));
  return textPtr;
}

UIBoxButtonComponent* AddBoxButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width,
    float Height,
    int NormalColor,
    int HoveredColor,
    int PressedColor
) {
  auto button = std::make_unique<UIBoxButtonComponent>(Width, Height, NormalColor, HoveredColor, PressedColor);
  UIBoxButtonComponent* buttonPtr = button.get();
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(buttonPtr);
  }

  Owner->AddComponent(std::move(button));
  AddText(
      Owner,
      buttonPtr,
      Label,
      0xFFFFFF,
      22,
      {Width, Height},
      EUIAnchor::MiddleCenter,
      {0.5f, 0.5f},
      {0.0f, 0.0f}
  );

  return buttonPtr;
}

UIToggleButtonComponent* AddToggleButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width,
    float Height
) {
  auto button = std::make_unique<UIToggleButtonComponent>(Width, Height, ReadyOnColor, ReadyOffColor);
  UIToggleButtonComponent* buttonPtr = button.get();
  buttonPtr->SetPivot({0.5f, 0.5f});
  buttonPtr->SetColors(
      ReadyOnColor,
      ReadyOnHoveredColor,
      ReadyOnPressedColor,
      ReadyOffColor,
      ReadyOffHoveredColor,
      ReadyOffPressedColor
  );
  if (Container) {
    Container->AddItem(buttonPtr);
  }

  Owner->AddComponent(std::move(button));
  AddText(
      Owner,
      buttonPtr,
      Label,
      0xFFFFFF,
      22,
      {Width, Height},
      EUIAnchor::MiddleCenter,
      {0.5f, 0.5f},
      {0.0f, 0.0f}
  );

  return buttonPtr;
}
}  // namespace

void WLobbyHUD::SetLobbyController(PC_Lobby* InLobbyController) {
  LobbyController = InLobbyController;
}

void WLobbyHUD::BeginPlay() {
  AWidgetBase::BeginPlay();

  auto playerListBox = std::make_unique<MUIVerticalBoxComponent>();
  m_PlayerListBox = playerListBox.get();
  m_PlayerListBox->SetAnchor(EUIAnchor::TopLeft);
  m_PlayerListBox->SetPivot({0.0f, 0.0f});
  m_PlayerListBox->SetWidgetSize({PlayerRowWidth, 1.0f});
  m_PlayerListBox->SetAnchoredPosition({36.0f, 36.0f});
  m_PlayerListBox->SetSpacing(8.0f);
  AddComponent(std::move(playerListBox));

  auto mapInfoBox = std::make_unique<MUIVerticalBoxComponent>();
  m_MapInfoBox = mapInfoBox.get();
  m_MapInfoBox->SetAnchor(EUIAnchor::TopRight);
  m_MapInfoBox->SetPivot({1.0f, 0.0f});
  m_MapInfoBox->SetWidgetSize({360.0f, 1.0f});
  m_MapInfoBox->SetAnchoredPosition({-44.0f, 50.0f});
  m_MapInfoBox->SetSpacing(12.0f);
  AddComponent(std::move(mapInfoBox));

  m_SelectedMapText = AddText(
      this,
      nullptr,
      "Selected Map: -",
      0xFFFFFF,
      24,
      {360.0f, 34.0f},
      EUIAnchor::MiddleCenter,
      {0.5f, 0.5f},
      {0.0f, 0.0f}
  );
  m_MapInfoBox->AddItem(m_SelectedMapText);

  m_MapSelectButton = AddBoxButton(
      this,
      m_MapInfoBox,
      "Change Map",
      MapButtonWidth,
      MapButtonHeight,
      ButtonNormalColor,
      ButtonHoveredColor,
      ButtonPressedColor
  );

  auto actionBox = std::make_unique<MUIVerticalBoxComponent>();
  m_ActionBox = actionBox.get();
  m_ActionBox->SetAnchor(EUIAnchor::BottomCenter);
  m_ActionBox->SetPivot({0.5f, 1.0f});
  m_ActionBox->SetWidgetSize({ActionButtonWidth, 1.0f});
  m_ActionBox->SetAnchoredPosition({0.0f, -72.0f});
  m_ActionBox->SetSpacing(12.0f);
  AddComponent(std::move(actionBox));

  m_ReadyToggle = AddToggleButton(this, m_ActionBox, "Ready", ActionButtonWidth, ActionButtonHeight);
  m_StartGameButton = AddBoxButton(
      this,
      m_ActionBox,
      "Start Game",
      ActionButtonWidth,
      ActionButtonHeight,
      ButtonDisabledColor,
      ButtonDisabledColor,
      ButtonDisabledColor
  );

  const bool bIsHost = LobbyController && LobbyController->GetWorld() && LobbyController->GetWorld()->IsServer();
  m_LeaveButton = AddBoxButton(
      this,
      nullptr,
      bIsHost ? "Disband Lobby" : "Leave Lobby",
      LeaveButtonWidth,
      LeaveButtonHeight,
      LeaveNormalColor,
      LeaveHoveredColor,
      LeavePressedColor
  );
  m_LeaveButton->SetAnchor(EUIAnchor::BottomLeft);
  m_LeaveButton->SetPivot({0.0f, 1.0f});
  m_LeaveButton->SetAnchoredPosition({44.0f, -44.0f});

  if (m_ReadyToggle) {
    m_ReadyToggle->OnToggled = [this](bool bIsOn) {
      if (!LobbyController) {
        return;
      }
      if (auto* localState = LobbyController->FindLocalPlayerState()) {
        localState->SetReady(bIsOn);
      }
    };
  }

  if (m_MapSelectButton) {
    m_MapSelectButton->OnPressed = [this]() {
      if (LobbyController && LobbyController->GetWorld() && LobbyController->GetWorld()->IsServer()) {
        LobbyController->ShowMapSelectDialog();
      }
    };
  }

  if (m_StartGameButton) {
    m_StartGameButton->OnPressed = [this]() {
      if (LobbyController && LobbyController->GetWorld() && LobbyController->GetWorld()->IsServer() && CanStartGame()) {
        LobbyController->StartGame();
      }
    };
  }

  if (m_LeaveButton) {
    m_LeaveButton->OnPressed = [this]() {
      if (LobbyController) {
        LobbyController->LeaveLobby();
      }
    };
  }

  m_bLastHostMode = bIsHost;
  if (m_MapSelectButton) {
    m_MapSelectButton->SetVisibility(bIsHost);
  }
  if (m_StartGameButton) {
    m_StartGameButton->SetVisibility(bIsHost);
  }

  UpdatePlayerList();
  UpdateMapInfo();
  UpdateLocalReadyState();
  UpdateStartGameState();
  RebuildNavigation();
  SetFocusedButton(m_ReadyToggle);
}

void WLobbyHUD::OnUpdate(float DeltaTime) {
  AWidgetBase::OnUpdate(DeltaTime);

  if (!LobbyController || !LobbyController->GetWorld()) {
    return;
  }

  if (LobbyController->FindHostPlayerState()) {
    LobbyController->ApplyHostLobbyOptions();
  }

  const bool bIsHost = LobbyController->GetWorld()->IsServer();
  if (bIsHost != m_bLastHostMode) {
    m_bLastHostMode = bIsHost;
    if (m_MapSelectButton) {
      m_MapSelectButton->SetVisibility(bIsHost);
    }
    if (m_StartGameButton) {
      m_StartGameButton->SetVisibility(bIsHost);
    }
    RebuildNavigation();
  }

  UpdatePlayerList();
  UpdateMapInfo();
  UpdateLocalReadyState();
  UpdateStartGameState();
}

void WLobbyHUD::Draw() {
  AWidgetBase::Draw();
}

void WLobbyHUD::UpdatePlayerList() {
  if (!LobbyController || !m_PlayerListBox) {
    return;
  }

  const std::vector<ALobbyPlayerState*> states = LobbyController->GetPlayerStates();
  const int playerCount = static_cast<int>(states.size());

  while (static_cast<int>(m_PlayerRows.size()) < playerCount) {
    auto rowRoot = std::make_unique<MUIWidgetComponent>();
    MUIWidgetComponent* rowRootPtr = rowRoot.get();
    rowRootPtr->SetWidgetSize({PlayerRowWidth, PlayerRowHeight});
    rowRootPtr->SetPivot({0.5f, 0.5f});
    m_PlayerListBox->AddItem(rowRootPtr);
    AddComponent(std::move(rowRoot));

    UITextComponent* statusText = AddText(
        this,
        rowRootPtr,
        "Not Ready",
        NotReadyTextColor,
        20,
        {112.0f, PlayerRowHeight},
        EUIAnchor::MiddleLeft,
        {0.0f, 0.5f},
        {0.0f, 0.0f}
    );

    UITextComponent* nameText = AddText(
        this,
        rowRootPtr,
        "Player",
        0xFFFFFF,
        20,
        {240.0f, PlayerRowHeight},
        EUIAnchor::MiddleLeft,
        {0.0f, 0.5f},
        {124.0f, 0.0f}
    );

    m_PlayerRows.push_back({rowRootPtr, statusText, nameText});
  }

  while (static_cast<int>(m_PlayerRows.size()) > playerCount) {
    FPlayerRow& row = m_PlayerRows.back();
    if (row.RowRoot) {
      m_PlayerListBox->RemoveItem(row.RowRoot);
      row.RowRoot->DestroyComponent();
    }
    m_PlayerRows.pop_back();
  }

  for (int index = 0; index < playerCount; ++index) {
    ALobbyPlayerState* state = states[index];
    FPlayerRow& row = m_PlayerRows[index];
    const bool bReady = state && state->IsReady();
    const std::string playerName = state && !state->GetPlayerName().empty()
                                       ? state->GetPlayerName()
                                       : "Player " + std::to_string(index + 1);

    if (row.StatusText) {
      row.StatusText->SetText(bReady ? "Ready" : "Not Ready");
      row.StatusText->SetColor(bReady ? ReadyTextColor : NotReadyTextColor);
    }
    if (row.NameText) {
      row.NameText->SetText(playerName);
    }
  }

  m_LastPlayerCount = playerCount;
}

void WLobbyHUD::UpdateMapInfo() {
  if (!LobbyController || !m_SelectedMapText) {
    return;
  }

  const std::string selectedLevelPath = LobbyController->GetSelectedLevelPath();
  if (selectedLevelPath == m_LastMapPath) {
    return;
  }

  m_LastMapPath = selectedLevelPath;
  const std::string selectedMapName = FindMapDisplayName(selectedLevelPath);
  m_SelectedMapText->SetText("Selected Map: " + selectedMapName);
}

void WLobbyHUD::UpdateLocalReadyState() {
  if (!LobbyController || !m_ReadyToggle) {
    return;
  }

  ALobbyPlayerState* localState = LobbyController->FindLocalPlayerState();
  if (!localState) {
    return;
  }

  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    if (!gi->player_name.empty() && localState->GetPlayerName() != gi->player_name) {
      localState->SetPlayerName(gi->player_name);
    }
  }

  m_ReadyToggle->SetIsOn(localState->IsReady(), false);
}

void WLobbyHUD::UpdateStartGameState() {
  if (!m_StartGameButton) {
    return;
  }

  const bool bCanStartGame = CanStartGame();
  if (bCanStartGame == m_bLastCanStartGame) {
    return;
  }

  m_bLastCanStartGame = bCanStartGame;
  if (bCanStartGame) {
    m_StartGameButton->SetColors(ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor);
  } else {
    m_StartGameButton->SetColors(ButtonDisabledColor, ButtonDisabledColor, ButtonDisabledColor);
  }
}

void WLobbyHUD::RebuildNavigation() {
  if (m_ActionBox) {
    m_ActionBox->BuildNavigation();
  }

  if (m_ReadyToggle) {
    m_ReadyToggle->Navigation.Left = m_LeaveButton;
    m_ReadyToggle->Navigation.Right = m_bLastHostMode ? m_MapSelectButton : nullptr;
  }
  if (m_StartGameButton) {
    m_StartGameButton->Navigation.Left = m_LeaveButton;
    m_StartGameButton->Navigation.Right = m_bLastHostMode ? m_MapSelectButton : nullptr;
    m_StartGameButton->Navigation.Up = m_ReadyToggle;
  }
  if (m_LeaveButton) {
    m_LeaveButton->Navigation.Right = m_ReadyToggle;
  }
  if (m_MapSelectButton) {
    m_MapSelectButton->Navigation.Left = m_ReadyToggle;
    m_MapSelectButton->Navigation.Down = m_ReadyToggle;
  }
}

bool WLobbyHUD::CanStartGame() const {
  if (!LobbyController || !LobbyController->GetWorld() || !LobbyController->GetWorld()->IsServer()) {
    return false;
  }

  const std::vector<ALobbyPlayerState*> states = LobbyController->GetPlayerStates();
  if (states.empty()) {
    return false;
  }

  return std::all_of(states.begin(), states.end(), [](const ALobbyPlayerState* state) {
    return state && state->IsReady();
  });
}
