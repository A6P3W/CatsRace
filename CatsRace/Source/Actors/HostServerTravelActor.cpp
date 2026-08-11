#include "Actors/HostServerTravelActor.h"

#include <imgui.h>

#include <utility>

#include "PathResolver.h"
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
  if (SelectedLevelPath.empty()) {
    ImGui::TextUnformatted("Level: Not selected");
  } else {
    ImGui::Text("Level: %s", SelectedLevelPath.c_str());
  }

  const bool bSubmitted = ImGui::InputText(
      "Level path", LevelPathInput.data(), LevelPathInput.size(), ImGuiInputTextFlags_EnterReturnsTrue
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
  ImGui::End();
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
