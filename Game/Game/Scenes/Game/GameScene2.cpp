#include "GameScene2.h"

#include <SceneManager.h>

#include "Core/GameSceneIds.h"
#include "Objects/Items/HeldSpeedItem.h"
#include "Scenes/Game/LapCheckpoint.h"

REGISTER_GAME_MODE(AGameScene2)

AGameScene2::AGameScene2() {}  

void AGameScene2::BeginPlay() {
  AGameSceneBase::BeginPlay();  
  auto* cp0 = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{-2098.0f, 3600.0f});
  cp0->SetCheckpointIndex(0);

  auto* cp1 = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{-2098.0f, 3802.0f});
  cp1->SetCheckpointIndex(1);



  // ゴールライン（通常CPの総数を渡す）
  auto* lapLine = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{-2098.0f, 4002.0f});
  lapLine->SetIsLapLine(true);
  lapLine->SetTotalCheckpoints(2);


 
}

void AGameScene2::OpenCurrentScene() {
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Game2);
}

