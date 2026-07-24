#include <DxLib.h>
#include <Windows.h>

#include "Application.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "EOSCoreManager.h"
#include "NetworkManager.h"
#include "SceneManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Clear/PC_Clear.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Game/GameScene2.h"
#include "Scenes/Game/PC_Game.h"
#include "Scenes/Loading/LoadingScene.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "Scenes/Menu/MenuScene.h"
void SetupGame() {
  EOSCoreManager::GetInstance().InitializeOnlineServices();
  NetworkManager::GetInstance().SetTransportType(ENetworkTransportType::ENet);
  auto& SM = SceneManager::GetInstance();
  SM.SetGameInstance<GI_main>();
  SM.RegisterLevelPath(GameSceneIds::Loading, "Resources/LoadingScene.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Menu, "Resources/MenuScene.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Clear, "Resources/ClearScene.BLevel");
  SM.SetStartupLevelPath("Resources/LoadingScene.BLevel");
  SetMouseDispFlag(1);
}

int WINAPI
WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, LPSTR CommandLine, int ShowCommand) {
  (void)Instance;
  (void)PreviousInstance;
  (void)CommandLine;
  (void)ShowCommand;
  Application::SetGameSetupCallback(&SetupGame);
  Application App;
  return App.Run() ? 0 : 1;
}
