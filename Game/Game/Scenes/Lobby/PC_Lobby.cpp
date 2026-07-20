#include "Scenes/Lobby/PC_Lobby.h"

#include <algorithm>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Core/MapData.h"
#include "NetworkManager.h"
#include "OnlinePlayManager.h"
#include "SceneManager.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Lobby/UI/WLeaveLobbyConfirmDialog.h"
#include "Scenes/Lobby/UI/WLobbyHUD.h"
#include "Scenes/Lobby/UI/WMapSelectDialog.h"
#include "UIManager.h"
#include "World.h"

REGISTER_ACTOR(PC_Lobby)

PC_Lobby::PC_Lobby() { SetUpdateableAnytime(true); }

void PC_Lobby::BeginPlay() {
  APlayerController::BeginPlay();
  if (bIsLocallyControlled) {
    SetInputMode(EInputMode::UIOnly);
    SetupInputMappings();

    m_LobbyHUD = GetWorld()->SpawnActor<WLobbyHUD>();
    m_LobbyHUD->SetLobbyController(this);
    UIManager::GetInstance()->AddWidget(m_LobbyHUD);
    UIManager::GetInstance()->SetFocusedWidget(m_LobbyHUD);
    // SoundManager 経由で直接再生
    if (auto* sm = GetWorld()->GetSoundManager()) {
      sm->PlaySE("Resources/images/moving-v2.mp3", false);
    }
  }
}

void PC_Lobby::OnUpdate(float DeltaTime) {
  APlayerController::OnUpdate(DeltaTime);

  if (!bIsLocallyControlled) {
    return;
  }

  auto* localState = FindLocalPlayerState();
  auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (localState && gi && !gi->player_name.empty() &&
      localState->GetPlayerName() != gi->player_name) {
    localState->SetPlayerName(gi->player_name);
  }
}

std::vector<ALobbyPlayerState*> PC_Lobby::GetPlayerStates() {
  std::vector<ALobbyPlayerState*> states;
  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return states;
  }

  for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    if (auto* state = dynamic_cast<ALobbyPlayerState*>(actorPtr.get())) {
      if (!state->IsPendingDestroy()) {
        states.push_back(state);
      }
    }
  }

  std::sort(
      states.begin(), states.end(), [](const ALobbyPlayerState* a, const ALobbyPlayerState* b) {
        return a->OwnerConnectionId < b->OwnerConnectionId;
      }
  );
  return states;
}

ALobbyPlayerState* PC_Lobby::FindLocalPlayerState() {
  const FNetworkConnectionId localId = NetworkManager::GetInstance().GetLocalConnectionId();
  for (auto* state : GetPlayerStates()) {
    if (!state) {
      continue;
    }
    if ((GetWorld()->IsServer() && state->OwnerConnectionId == 0) ||
        state->OwnerConnectionId == localId || state->bIsLocallyControlled) {
      return state;
    }
  }
  return nullptr;
}

ALobbyPlayerState* PC_Lobby::FindHostPlayerState() { return FindPlayerState(0); }

ALobbyPlayerState* PC_Lobby::FindPlayerState(FNetworkConnectionId ConnectionId) {
  for (auto* state : GetPlayerStates()) {
    if (state && state->OwnerConnectionId == ConnectionId) {
      return state;
    }
  }
  return nullptr;
}

int PC_Lobby::GetMaxPlayers() const {
  if (auto* lobbyScene = GetLobbyScene()) {
    return lobbyScene->MaxPlayers;
  }
  if (auto* hostState = const_cast<PC_Lobby*>(this)->FindHostPlayerState()) {
    return hostState->GetMaxPlayers();
  }
  return 0;
}

std::string PC_Lobby::GetSelectedLevelPath() const {
  if (auto* lobbyScene = GetLobbyScene()) {
    return lobbyScene->SelectedLevelPath;
  }
  if (auto* hostState = const_cast<PC_Lobby*>(this)->FindHostPlayerState()) {
    return hostState->GetSelectedLevelPath();
  }
  return AvailableMaps.empty() ? std::string{} : AvailableMaps.front().LevelPath;
}

void PC_Lobby::ApplyHostLobbyOptions() {
  auto* lobbyScene = GetLobbyScene();
  auto* hostState = FindHostPlayerState();
  if (!lobbyScene || !hostState) {
    return;
  }

  lobbyScene->SelectedLevelPath = hostState->GetSelectedLevelPath();
  lobbyScene->MaxPlayers = hostState->GetMaxPlayers();
}

void PC_Lobby::SetMaxPlayers(int InMaxPlayers) {
  auto* lobbyScene = GetLobbyScene();
  if (!lobbyScene) {
    return;
  }

  lobbyScene->MaxPlayers = InMaxPlayers;
  if (auto* host = lobbyScene->FindHostPlayerState()) {
    host->SetLobbyOptions(lobbyScene->SelectedLevelPath, lobbyScene->MaxPlayers);
  }
}

void PC_Lobby::ShowMapSelectDialog() {
  if (!bIsLocallyControlled || !GetWorld() || !GetWorld()->IsServer() || m_MapSelectDialog) {
    return;
  }

  m_MapSelectDialog = GetWorld()->SpawnActor<WMapSelectDialog>();
  m_MapSelectDialog->SetZOrderOffset(10);
  m_MapSelectDialog->OnMapSelected = [this](const std::string& LevelPath) {
    auto* dialog = m_MapSelectDialog;
    m_MapSelectDialog = nullptr;
    if (dialog) {
      dialog->Destroy();
    }

    if (!LevelPath.empty()) {
      if (auto* lobbyScene = GetLobbyScene()) {
        lobbyScene->SelectedLevelPath = LevelPath;
        if (auto* host = lobbyScene->FindHostPlayerState()) {
          host->SetLobbyOptions(lobbyScene->SelectedLevelPath, lobbyScene->MaxPlayers);
        }
      }
    }

    if (m_LobbyHUD) {
      UIManager::GetInstance()->SetFocusedWidget(m_LobbyHUD);
      m_LobbyHUD->FocusMapSelectButton();
    }
  };

  UIManager::GetInstance()->AddWidget(m_MapSelectDialog);
  UIManager::GetInstance()->SetFocusedWidget(m_MapSelectDialog);
}

void PC_Lobby::StartGame() {
  if (auto* lobbyScene = GetLobbyScene()) {
    lobbyScene->StartGame();
  }
}

void PC_Lobby::ShowLeaveLobbyConfirmDialog() {
  if (!bIsLocallyControlled || !GetWorld() || m_LeaveLobbyConfirmDialog) {
    return;
  }

  m_LeaveLobbyConfirmDialog = GetWorld()->SpawnActor<WLeaveLobbyConfirmDialog>();
  m_LeaveLobbyConfirmDialog->SetZOrderOffset(10);
  m_LeaveLobbyConfirmDialog->OnResult = [this](bool bLeaveLobby) {
    auto* dialog = m_LeaveLobbyConfirmDialog;
    m_LeaveLobbyConfirmDialog = nullptr;
    if (dialog) {
      dialog->Destroy();
    }

    if (bLeaveLobby) {
      LeaveLobby();
      return;
    }

    if (m_LobbyHUD) {
      UIManager::GetInstance()->SetFocusedWidget(m_LobbyHUD);
    }
  };

  UIManager::GetInstance()->AddWidget(m_LeaveLobbyConfirmDialog);
  UIManager::GetInstance()->SetFocusedWidget(m_LeaveLobbyConfirmDialog);
}

bool PC_Lobby::IsStartCountdownActive() const { return GetStartCountdownSeconds() >= 0; }

int PC_Lobby::GetStartCountdownSeconds() const {
  if (auto* hostState = const_cast<PC_Lobby*>(this)->FindHostPlayerState()) {
    return hostState->GetStartCountdownSeconds();
  }
  return -1;
}
void PC_Lobby::LeaveLobby() {
  auto returnToMenu = []() {
    NetworkManager::GetInstance().Disconnect();
    SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu, ENetMode::Standalone);
  };

  if (!OnlinePlayManager::GetInstance().IsInLobby()) {
    returnToMenu();
    return;
  }

  OnlinePlayManager::GetInstance().LeaveLobby([returnToMenu](const FOnlinePlayResult& Result) {
    if (Result.Success) {
      returnToMenu();
    }
  });
}

ALobbyScene* PC_Lobby::GetLobbyScene() const {
  auto* self = const_cast<PC_Lobby*>(this);
  if (!self->GetWorld()) {
    return nullptr;
  }
  return dynamic_cast<ALobbyScene*>(self->GetWorld()->GetGameMode());
}
