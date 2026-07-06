#include <DxLib.h>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "EOSCoreManager.h"
#include "SceneManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Clear/PC_Clear.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Game/PC_Game.h"
#include "Scenes/Loading/LoadingScene.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "Scenes/Menu/MenuScene.h"
void SetupGame() {
  EOSCoreManager::Get().InitializeOnlineServices();
  auto& SM = SceneManager::GetInstance();
  SM.SetGameInstance<GI_main>();
  SM.RegisterLevelPath(GameSceneIds::Loading, "Resources/LoadingScene.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Menu, "Resources/MenuScene.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Lobby, "Resources/LobbyScene.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Game01, "Resources/GameScene01.BLevel");
  SM.RegisterLevelPath(GameSceneIds::Clear, "Resources/ClearScene.BLevel");

  SM.SetStartupLevelPath("Resources/LoadingScene.BLevel");
  SetMouseDispFlag(1);
}
