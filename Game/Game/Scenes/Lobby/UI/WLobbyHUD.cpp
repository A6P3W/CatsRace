#include "Scenes/Lobby/UI/WLobbyHUD.h"

#include <DxLib.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>
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

constexpr FColor ButtonNormalColor{32, 38, 48};
constexpr FColor ButtonHoveredColor{30, 115, 190};
constexpr FColor ButtonPressedColor{15, 78, 140};
constexpr FColor ButtonDisabledColor{85, 90, 98};
constexpr FColor LeaveNormalColor{64, 50, 50};
constexpr FColor LeaveHoveredColor{150, 70, 70};
constexpr FColor LeavePressedColor{110, 45, 45};

std::string FindMapDisplayName(const std::string& LevelPath) {
  const auto it =
      std::find_if(AvailableMaps.begin(), AvailableMaps.end(), [&LevelPath](const FMapInfo& Map) {
        return Map.LevelPath == LevelPath;
      });
  return it == AvailableMaps.end() ? LevelPath : it->DisplayName;
}

UITextComponent* AddText(
    AWidgetBase* Owner,
    MUIWidgetComponent* Parent,
    const std::string& Text,
    const FColor& Color,
    int FontSize,
    const FVector2D& WidgetSize,
    EUIAnchor Anchor,
    const FVector2D& Pivot,
    const FVector2D& Position
) {
  auto* textPtr = NewObject<UITextComponent>(Owner);
  textPtr->SetText(Text);
  textPtr->SetColor(Color);
  textPtr->SetFontSize(FontSize);
  textPtr->AttachToComponent(Parent);
  textPtr->SetWidgetSize(WidgetSize);
  textPtr->SetAnchor(Anchor);
  textPtr->SetPivot(Pivot);
  textPtr->SetAnchoredPosition(Position);
  textPtr->RegisterComponent();
  return textPtr;
}

UIBoxButtonComponent* AddBoxButton(
    AWidgetBase* Owner,
    MUIVerticalBoxComponent* Container,
    const std::string& Label,
    float Width,
    float Height,
    const FColor& NormalColor,
    const FColor& HoveredColor,
    const FColor& PressedColor
) {
  auto* buttonPtr = NewObject<UIBoxButtonComponent>(Owner);
  buttonPtr->SetSize(Width, Height);
  buttonPtr->SetColors(NormalColor, HoveredColor, PressedColor);
  buttonPtr->SetPivot({0.5f, 0.5f});
  if (Container) {
    Container->AddItem(buttonPtr);
  }

  buttonPtr->RegisterComponent();
  AddText(
      Owner,
      buttonPtr,
      Label,
      FColor::White,
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

  m_PlayerListBox = NewObject<MUIVerticalBoxComponent>(this);
  m_PlayerListBox->SetAnchor(EUIAnchor::TopLeft);
  m_PlayerListBox->SetPivot({0.0f, 0.0f});
  m_PlayerListBox->SetWidgetSize({PlayerRowWidth, 1.0f});
  m_PlayerListBox->SetAnchoredPosition({36.0f, 36.0f});
  m_PlayerListBox->SetSpacing(8.0f);
  m_PlayerListBox->RegisterComponent();

  m_MapInfoBox = NewObject<MUIVerticalBoxComponent>(this);
  m_MapInfoBox->SetAnchor(EUIAnchor::TopRight);
  m_MapInfoBox->SetPivot({1.0f, 0.0f});
  m_MapInfoBox->SetWidgetSize({360.0f, 1.0f});
  m_MapInfoBox->SetAnchoredPosition({-44.0f, 50.0f});
  m_MapInfoBox->SetSpacing(12.0f);
  m_MapInfoBox->RegisterComponent();

  m_SelectedMapText = AddText(
      this,
      nullptr,
      "Selected Map: -",
      FColor::White,
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

  m_ActionBox = NewObject<MUIVerticalBoxComponent>(this);
  m_ActionBox->SetAnchor(EUIAnchor::BottomCenter);
  m_ActionBox->SetPivot({0.5f, 1.0f});
  m_ActionBox->SetWidgetSize({ActionButtonWidth, 1.0f});
  m_ActionBox->SetAnchoredPosition({0.0f, -72.0f});
  m_ActionBox->SetSpacing(12.0f);
  m_ActionBox->RegisterComponent();

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

  StartCountdownText = AddText(
      this, nullptr, "", FColor::White, 28, {420.0f, 40.0f},
      EUIAnchor::BottomCenter, {0.5f, 1.0f}, {0.0f, -144.0f}
  );
  StartCountdownText->SetVisibility(false);
  const bool bIsHost =
      LobbyController && LobbyController->GetWorld() && LobbyController->GetWorld()->IsServer();
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

  if (m_MapSelectButton) {
    m_MapSelectButton->SetOnPressed([this]() {
      if (LobbyController && LobbyController->GetWorld() &&
          LobbyController->GetWorld()->IsServer() && !LobbyController->IsStartCountdownActive()) {
        LobbyController->ShowMapSelectDialog();
      }
    });
  }

  if (m_StartGameButton) {
    m_StartGameButton->SetOnPressed([this]() {
      if (LobbyController && LobbyController->GetWorld() &&
          LobbyController->GetWorld()->IsServer() && !LobbyController->IsStartCountdownActive() && !LobbyController->GetPlayerStates().empty() && !LobbyController->GetSelectedLevelPath().empty()) {
        LobbyController->StartGame();
      }
    });
  }

  if (m_LeaveButton) {
    m_LeaveButton->SetOnPressed([this]() {
      if (LobbyController) {
        LobbyController->LeaveLobby();
      }
    });
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
  UpdateStartGameState();
  UpdateStartCountdown();
  RebuildNavigation();
  UpdateFocusForHostMode(bIsHost);
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
    UpdateFocusForHostMode(bIsHost);
  }

  UpdatePlayerList();
  UpdateMapInfo();
  UpdateStartGameState();
  UpdateStartCountdown();
}

void WLobbyHUD::Draw() { AWidgetBase::Draw(); }

void WLobbyHUD::UpdatePlayerList() {
  if (!LobbyController || !m_PlayerListBox) {
    return;
  }

  const std::vector<ALobbyPlayerState*> states = LobbyController->GetPlayerStates();
  const int playerCount = static_cast<int>(states.size());

  while (static_cast<int>(m_PlayerRows.size()) < playerCount) {
    auto* rowRootPtr = NewObject<MUIWidgetComponent>(this);
    rowRootPtr->SetWidgetSize({PlayerRowWidth, PlayerRowHeight});
    rowRootPtr->SetPivot({0.5f, 0.5f});
    m_PlayerListBox->AddItem(rowRootPtr);
    rowRootPtr->RegisterComponent();

    UITextComponent* statusText = AddText(
        this,
        rowRootPtr,
        "Player",
        FColor::White,
        20,
        {132.0f, PlayerRowHeight},
        EUIAnchor::MiddleLeft,
        {0.0f, 0.5f},
        {0.0f, 0.0f}
    );

    UITextComponent* nameText = AddText(
        this,
        rowRootPtr,
        "Player",
        FColor::White,
        20,
        {216.0f, PlayerRowHeight},
        EUIAnchor::MiddleLeft,
        {0.0f, 0.5f},
        {144.0f, 0.0f}
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
    const bool bIsHost = state && state->OwnerConnectionId == 0;
    const std::string playerName = state && !state->GetPlayerName().empty()
                                       ? state->GetPlayerName()
                                       : "Player " + std::to_string(index + 1);

    if (row.StatusText) {
      row.StatusText->SetText(bIsHost ? "Host" : "Player");
      row.StatusText->SetColor(FColor::White);
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

void WLobbyHUD::UpdateStartGameState() {
  if (!m_StartGameButton) {
    return;
  }

  const bool bCanStartGame = LobbyController && LobbyController->GetWorld() && LobbyController->GetWorld()->IsServer() && !LobbyController->IsStartCountdownActive() && !LobbyController->GetPlayerStates().empty() && !LobbyController->GetSelectedLevelPath().empty();
  if (bCanStartGame) {
    m_StartGameButton->SetColors(ButtonNormalColor, ButtonHoveredColor, ButtonPressedColor);
  } else {
    m_StartGameButton->SetColors(ButtonDisabledColor, ButtonDisabledColor, ButtonDisabledColor);
  }
}

void WLobbyHUD::UpdateStartCountdown() {
  if (!StartCountdownText || !LobbyController) {
    return;
  }
  const int seconds = LobbyController->GetStartCountdownSeconds();
  if (seconds == LastStartCountdownSeconds) {
    return;
  }
  LastStartCountdownSeconds = seconds;
  StartCountdownText->SetVisibility(seconds >= 0);
  StartCountdownText->SetText(
      seconds > 0 ? "Starting in " + std::to_string(seconds) + "..." : "Starting..."
  );
}
void WLobbyHUD::RebuildNavigation() {
  if (m_ActionBox) {
    m_ActionBox->BuildNavigation();
  }

  if (m_StartGameButton) {
    m_StartGameButton->Navigation.Left = m_LeaveButton;
    m_StartGameButton->Navigation.Right = m_bLastHostMode ? m_MapSelectButton : nullptr;
    m_StartGameButton->Navigation.Up = nullptr;
  }
  MUIButtonComponent* primaryActionButton =
      m_bLastHostMode ? static_cast<MUIButtonComponent*>(m_StartGameButton)
                      : nullptr;
  if (m_LeaveButton) {
    m_LeaveButton->Navigation.Right = primaryActionButton;
  }
  if (m_MapSelectButton) {
    m_MapSelectButton->Navigation.Left = primaryActionButton;
    m_MapSelectButton->Navigation.Down = primaryActionButton;
  }
}

void WLobbyHUD::UpdateFocusForHostMode(bool bIsHost) {
  SetFocusedButton(
      bIsHost ? static_cast<MUIButtonComponent*>(m_StartGameButton)
              : static_cast<MUIButtonComponent*>(m_LeaveButton)
  );
}

