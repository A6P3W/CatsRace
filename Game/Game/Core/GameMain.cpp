#include "SceneManager.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Menu/MenuScene.h"
#include "Scenes/Title/TitleScene.h"
#include "Scenes/Lobby/PC_Lobby.h"
#include "Scenes/Game/PC_Game.h"
#include "Scenes/Clear/PC_Clear.h"
#include <DxLib.h>

void SetupGame() {
	auto& SM = SceneManager::GetInstance();
	SM.SetGameInstance<GI_main>();
	SM.RegisterLevelPath(GameSceneIds::Title, "TitleScene.BLevel");
	SM.RegisterLevelPath(GameSceneIds::Menu, "MenuScene.BLevel");
	SM.RegisterLevelPath(GameSceneIds::Lobby, "LobbyScene.BLevel");
	SM.RegisterLevelPath(GameSceneIds::Game01, "GameScene01.BLevel");
	SM.RegisterLevelPath(GameSceneIds::Clear, "ClearScene.BLevel");

	SM.RegisterLocalPlayerControllerClass(GameSceneIds::Title, APlayerController::StaticClassName());
	SM.RegisterLocalPlayerControllerClass(GameSceneIds::Menu, APlayerController::StaticClassName());
	SM.RegisterLocalPlayerControllerClass(GameSceneIds::Lobby, PC_Lobby::StaticClassName());
	SM.RegisterLocalPlayerControllerClass(GameSceneIds::Game01, PC_Game::StaticClassName());
	SM.RegisterLocalPlayerControllerClass(GameSceneIds::Clear, PC_Clear::StaticClassName());

	SM.OpenSceneById(GameSceneIds::Title);
	SetMouseDispFlag(1);
}
