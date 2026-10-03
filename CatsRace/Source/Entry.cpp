#include <Windows.h>
#include <imgui.h>

#include "Application.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "EOSCoreManager.h"
#include "Gameplay/Race/Player/PC_Game.h"
#include "Gameplay/Race/Scenes/GameScene01.h"
#include "Gameplay/Race/Scenes/GameScene2.h"
#include "NetworkManager.h"
#include "PathResolver.h"
#include "SceneManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Clear/PC_Clear.h"
#include "Scenes/Loading/LoadingScene.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "Scenes/Menu/MenuScene.h"

namespace {
void SetupGame() {
  ImGui::SetCurrentContext(static_cast<ImGuiContext*>(Application::GetImGuiContext()));
  EOSCoreManager::GetInstance().InitializeOnlineServices();
  NetworkManager::GetInstance().SetTransportType(ENetworkTransportType::EOSP2P);

  auto& sceneManager = SceneManager::GetInstance();
  sceneManager.SetGameInstance<GI_main>();
  sceneManager.RegisterLevelPath(GameSceneIds::Loading, "/Game/LoadingScene.BLevel");
  sceneManager.RegisterLevelPath(GameSceneIds::Menu, "/Game/MenuScene.BLevel");
  sceneManager.RegisterLevelPath(GameSceneIds::Clear, "/Game/ClearScene.BLevel");
  sceneManager.SetStartupLevelPath("/Game/LoadingScene.BLevel");
}
}  // namespace

int WINAPI
WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, LPSTR CommandLine, int ShowCommand) {
  PathResolver::SetProjectRoot(BROCCOLI_PROJECT_ROOT);
  PathResolver::SetGameName("CatsRace");

  Application::SetGameSetupCallback(&SetupGame);
  Application App;
  return App.Run() ? 0 : 1;
}
