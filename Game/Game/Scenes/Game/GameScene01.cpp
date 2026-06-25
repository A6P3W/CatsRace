#include "GameScene01.h"
#include "Core/GameSceneIds.h"
#include <SceneManager.h>

AGameScene01::AGameScene01()
	: AGameSceneBase("GameScene01", "GameScene01.BLevel", FVector2D{ -2800.0f, -1700.0f })
{
}

void AGameScene01::OpenCurrentScene()
{
	SceneManager::GetInstance().OpenSceneById(GameSceneIds::Game01);
}