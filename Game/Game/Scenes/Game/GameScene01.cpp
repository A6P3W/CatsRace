#include "GameScene01.h"

#include <SceneManager.h>

#include "Core/GameSceneIds.h"

REGISTER_GAME_MODE(AGameScene01)

AGameScene01::AGameScene01() {}

void AGameScene01::OpenCurrentScene() {
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Game01);
}