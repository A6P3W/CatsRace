#include "SceneManager.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Lobby/LobbyScene.h"
#include "Scenes/Menu/MenuScene.h"
#include "Scenes/Title/TitleScene.h"
#include <DxLib.h>

void SetupGame() {
	auto& SM = SceneManager::GetInstance();
	SM.SetGameInstance<GI_main>();
	SM.RegisterScene<ATitleScene>(GameSceneIds::Title);
	SM.RegisterScene<AMenuScene>(GameSceneIds::Menu);
	SM.RegisterScene<ALobbyScene>(GameSceneIds::Lobby);
	SM.RegisterScene<AGameScene01>(GameSceneIds::Game01);
	SM.RegisterScene<AClearScene>(GameSceneIds::Clear);
	SM.OpenSceneById(GameSceneIds::Title);
	SetMouseDispFlag(1);
}