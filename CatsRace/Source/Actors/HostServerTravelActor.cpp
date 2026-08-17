#include "Actors/HostServerTravelActor.h"

#include <imgui.h>

#include <utility>

#include "ActorManager.h"
#include "Core/PlayerColorPalette.h"
#include "PathResolver.h"
#include "Scenes/Game/Player.h"
#include "World.h"

REGISTER_ACTOR(AHostServerTravelActor)

namespace {
constexpr const char* GameResourcePrefix = "/Game/";
constexpr const char* BLevelJsonExtension = ".BLevel.json";
}  // namespace

AHostServerTravelActor::AHostServerTravelActor() : StatusText("Ready") { bReplicates = false; }

void AHostServerTravelActor::SpawnForListenServer(World& InWorld) {
  if (InWorld.IsListenServer()) {
    InWorld.SpawnActor<AHostServerTravelActor>();
  }
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
