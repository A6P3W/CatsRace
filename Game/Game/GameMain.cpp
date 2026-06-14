#include "SceneManager.h"
#include "Objects/TitleScene.h"
void SetupGame() {
	SceneManager::GetInstance().OpenScene<ATitleScene>();
}
