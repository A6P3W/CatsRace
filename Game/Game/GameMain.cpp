#include "SceneManager.h"
#include "Objects/TitleScene.h"
#include "Objects/GI_main.h"
#include <DxLib.h>

void SetupGame() {
	auto &SM = SceneManager::GetInstance();
	SM.OpenScene<ATitleScene>();
	SM.SetGameInstance<GI_main>();
	SetMouseDispFlag(0);
}
