#include "SceneManager.h"
#include "Objects/GameScene01.h"
void SetupGame() {
	SceneManager::GetInstance().OpenScene<AGameScene01>();
}
