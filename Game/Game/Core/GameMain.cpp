#include "SceneManager.h"
#include "Scenes/Title/TitleScene.h"
#include "Core/GI_main.h"
#include <DxLib.h>

void SetupGame() {
	auto &SM = SceneManager::GetInstance();
	SM.OpenScene<ATitleScene>();
	SM.SetGameInstance<GI_main>();
	SetMouseDispFlag(0);
}
