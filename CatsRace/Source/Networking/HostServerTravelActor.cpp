#include "Actors/HostServerTravelActor.h"

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <utility>

#include "ActorManager.h"
#include "Core/GI_main.h"
#include "Core/PlayerColorPalette.h"
#include "PathResolver.h"
#include "SceneManager.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "World.h"

REGISTER_ACTOR(AHostServerTravelActor)

namespace {
constexpr const char* GameResourcePrefix = "/Game/";
constexpr const char* BLevelJsonExtension = ".BLevel.json";
}  // namespace

AHostServerTravelActor::AHostServerTravelActor() : StatusText("Ready") { bReplicates = false; }

void AHostServerTravelActor::SpawnForListenServer(World& InWorld) {
#if !defined(_RELEASE)
  if (InWorld.IsListenServer()) {
    InWorld.SpawnActor<AHostServerTravelActor>();
  }
#else
  (void)InWorld;
#endif
}

void AHostServerTravelActor::Draw() {
  World* CurrentWorld = GetWorld();
  if (!CurrentWorld || !CurrentWorld->IsListenServer()) {
    return;
  }

  ImGui::Begin("Server Travel");
  InitializePlayerEntries();
  if (SelectedLevelPath.empty()) {
    ImGui::TextUnformatted("Level: Not selected");
  } else {
    ImGui::Text("Level: %s", SelectedLevelPath.c_str());
  }

  const bool bSubmitted = ImGui::InputText(
      "Level path",
      LevelPathInput.data(),
      LevelPathInput.size(),
      ImGuiInputTextFlags_EnterReturnsTrue
  );
  ImGui::SameLine();
  if (ImGui::Button("Set Level") || bSubmitted) {
    SetLevelPathFromInput();
  }
  ImGui::SameLine();
  if (ImGui::Button("ServerTravel")) {
    ExecuteServerTravel();
  }

  ImGui::Text("Status: %s", StatusText.c_str());

  if (auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    bool BoothMode = GameInstance->BoothMode;
    if (ImGui::Checkbox("Booth mode", &BoothMode)) {
      GameInstance->BoothMode = BoothMode;
    }
  }

  DrawLobbyPlayerEntries();

  ImGui::Separator();
  ImGui::TextUnformatted("Player Speed");
  for (const FPlayerSpeedEntry& Entry : PlayerEntries) {
    APlayer* Player = Entry.Player;
    if (!Player || Player->IsPendingDestroy()) {
      continue;
    }

    FColor PlayerColor = FColor::White;
    const uint8_t ColorIndex = Player->GetPlayerColorIndex();
    if (ColorIndex < PlayerColorPalette.size()) {
      PlayerColor = PlayerColorPalette[ColorIndex];
    }
    const ImVec4 TextColor{
        PlayerColor.R / 255.0f,
        PlayerColor.G / 255.0f,
        PlayerColor.B / 255.0f,
        PlayerColor.A / 255.0f,
    };

    ImGui::PushID(Player);
    ImGui::PushStyleColor(ImGuiCol_Text, TextColor);
    ImGui::TextUnformatted(Player->GetPlayerName().c_str());
    ImGui::PopStyleColor();
    ImGui::SameLine();

    float Multiplier = Player->GetSpeedMultiplier();
    if (ImGui::SliderFloat(
            "##SpeedMultiplier",
            &Multiplier,
            APlayer::MinSpeedMultiplier,
            APlayer::MaxSpeedMultiplier,
            "%.2fx"
        )) {
      Player->SetSpeedMultiplier(Multiplier);
    }
    ImGui::PopID();
  }
  ImGui::End();
}

void AHostServerTravelActor::RefreshLobbyPlayerEntries() {
  World* CurrentWorld = GetWorld();
  if (!CurrentWorld || !CurrentWorld->GetActorManager() ||
      !dynamic_cast<ALobbyScene*>(CurrentWorld->GetGameMode())) {
    LobbyPlayerEntries.clear();
    return;
  }

  std::vector<ALobbyPlayerState*> States;
  for (const auto& ActorPtr : CurrentWorld->GetActorManager()->GetAllActors()) {
    auto* State = ActorPtr ? dynamic_cast<ALobbyPlayerState*>(ActorPtr.get()) : nullptr;
    if (State && !State->IsPendingDestroy()) {
      States.push_back(State);
    }
  }
  std::sort(States.begin(), States.end(), [](const auto* A, const auto* B) {
    return A->OwnerConnectionId < B->OwnerConnectionId;
  });

  std::vector<FLobbyPlayerEntry> RefreshedEntries;
  RefreshedEntries.reserve(States.size());
  for (ALobbyPlayerState* State : States) {
    const auto Existing = std::find_if(
        LobbyPlayerEntries.begin(),
        LobbyPlayerEntries.end(),
        [State](const FLobbyPlayerEntry& Entry) {
          return Entry.ConnectionId == State->OwnerConnectionId;
        }
    );

    FLobbyPlayerEntry Entry;
    if (Existing != LobbyPlayerEntries.end()) {
      Entry = std::move(*Existing);
    } else {
      Entry.ConnectionId = State->OwnerConnectionId;
    }
    Entry.PlayerState = State;
    if (Entry.LastObservedName != State->GetPlayerName()) {
      SetLobbyPlayerNameInput(Entry, State->GetPlayerName());
    }
    RefreshedEntries.push_back(std::move(Entry));
  }
  LobbyPlayerEntries = std::move(RefreshedEntries);
}

void AHostServerTravelActor::DrawLobbyPlayerEntries() {
  RefreshLobbyPlayerEntries();
  if (LobbyPlayerEntries.empty()) {
    return;
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Lobby Players");
  for (FLobbyPlayerEntry& Entry : LobbyPlayerEntries) {
    ALobbyPlayerState* State = Entry.PlayerState;
    if (!State || State->IsPendingDestroy()) {
      continue;
    }

    ImGui::PushID(static_cast<int>(Entry.ConnectionId));
    ImGui::Text(
        "[%u] %s: %s",
        Entry.ConnectionId,
        Entry.ConnectionId == 0 ? "Host" : "Player",
        State->GetPlayerName().c_str()
    );
    ImGui::InputText("##PlayerName", Entry.NameInput.data(), Entry.NameInput.size());
    ImGui::SameLine();

    const std::string NewName = Entry.NameInput.data();
    const bool bCanChange = !NewName.empty();
    ImGui::BeginDisabled(!bCanChange);
    if (ImGui::Button("Change Name") && bCanChange) {
      if (HasDuplicateBoothName(Entry.ConnectionId, NewName)) {
        LobbyPlayerStatusText = "Booth mode requires unique player names.";
      } else {
        State->SetPlayerName(NewName);
        Entry.LastObservedName = NewName;
        LobbyPlayerStatusText = "Changed player " + std::to_string(Entry.ConnectionId) + " name.";
      }
    }
    ImGui::EndDisabled();
    ImGui::PopID();
  }

  if (!LobbyPlayerStatusText.empty()) {
    ImGui::Text("Lobby status: %s", LobbyPlayerStatusText.c_str());
  }
}

void AHostServerTravelActor::SetLobbyPlayerNameInput(
    FLobbyPlayerEntry& Entry, const std::string& Name
) {
  Entry.NameInput.fill('\0');
  const size_t CopyLength = (std::min)(Name.size(), Entry.NameInput.size() - 1);
  std::memcpy(Entry.NameInput.data(), Name.data(), CopyLength);
  Entry.LastObservedName = Name;
}

bool AHostServerTravelActor::HasDuplicateBoothName(
    FNetworkConnectionId TargetConnectionId, const std::string& Name
) const {
  const auto* GameInstance = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!GameInstance || !GameInstance->BoothMode) {
    return false;
  }

  return std::any_of(
      LobbyPlayerEntries.begin(),
      LobbyPlayerEntries.end(),
      [TargetConnectionId, &Name](const FLobbyPlayerEntry& Entry) {
        return Entry.ConnectionId != TargetConnectionId && Entry.PlayerState &&
               !Entry.PlayerState->IsPendingDestroy() && Entry.PlayerState->GetPlayerName() == Name;
      }
  );
}

void AHostServerTravelActor::InitializePlayerEntries() {
  World* CurrentWorld = GetWorld();
  if (bPlayerEntriesInitialized || !CurrentWorld || !CurrentWorld->GetActorManager()) {
    return;
  }

  for (const auto& ActorPtr : CurrentWorld->GetActorManager()->GetAllActors()) {
    APlayer* Player = ActorPtr ? dynamic_cast<APlayer*>(ActorPtr.get()) : nullptr;
    if (!Player || Player->IsPendingDestroy()) {
      continue;
    }

    PlayerEntries.push_back({Player});
  }

  bPlayerEntriesInitialized = true;
}

void AHostServerTravelActor::SetLevelPathFromInput() {
  const std::string InputPath = LevelPathInput.data();
  if (InputPath.empty()) {
    StatusText = "Enter a level path first.";
    return;
  }

  std::string NormalizedPath;
  if (!NormalizeSelectedLevelPath(InputPath, NormalizedPath)) {
    SelectedLevelPath.clear();
    StatusText = "Selected level must be under /Game/.";
    return;
  }

  SelectedLevelPath = std::move(NormalizedPath);
  StatusText = "Level selected.";
}

void AHostServerTravelActor::ExecuteServerTravel() {
  World* CurrentWorld = GetWorld();
  if (!CurrentWorld || !CurrentWorld->IsListenServer()) {
    StatusText = "ServerTravel is available only on the listen server.";
    return;
  }
  if (SelectedLevelPath.empty()) {
    StatusText = "Select a level first.";
    return;
  }

  StatusText = CurrentWorld->ServerTravel(SelectedLevelPath) ? "ServerTravel queued."
                                                             : "ServerTravel failed.";
}

bool AHostServerTravelActor::NormalizeSelectedLevelPath(
    const std::string& InputPath, std::string& OutLevelPath
) const {
  OutLevelPath = PathResolver::SanitizeResourcePath(InputPath);
  if (OutLevelPath.rfind(GameResourcePrefix, 0) != 0) {
    return false;
  }

  if (OutLevelPath.ends_with(BLevelJsonExtension)) {
    OutLevelPath.resize(OutLevelPath.size() - 5);
  }
  return true;
}
