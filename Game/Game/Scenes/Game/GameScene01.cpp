#include "GameScene01.h"

#include <SceneManager.h>

#include "Core/GameSceneIds.h"

REGISTER_GAME_MODE(AGameScene01)

AGameScene01::AGameScene01()
    : AGameSceneBase("GameScene01", "GameScene01.BLevel", FVector2D{-2800.0f, -1700.0f}) {}

void AGameScene01::OpenCurrentScene() {
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Game01);
}