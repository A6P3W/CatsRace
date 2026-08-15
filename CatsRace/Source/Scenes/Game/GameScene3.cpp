#include "GameScene3.h"

#include <SceneManager.h>

#include "Core/GameSceneIds.h"
#include "Objects/Items/HeldSpeedItem.h"
#include "Scenes/Game/LapCheckpoint.h"

REGISTER_GAME_MODE(AGameScene3)

AGameScene3::AGameScene3() {}

void AGameScene3::BeginPlay() {
  AGameSceneBase::BeginPlay();
  auto* cp0 = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{-4968.0f, -2829.0f});
  cp0->SetCheckpointIndex(0);

  auto* cp1 = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{3916.0f, 2520.0f});
  cp1->SetCheckpointIndex(1);

  // ゴールライン（通常CPの総数を渡す）
  auto* lapLine = GetWorld()->SpawnActor<ALapCheckpoint>(FVector2D{-2098.0f, 4002.0f});
  lapLine->SetIsLapLine(true);
  lapLine->SetTotalCheckpoints(2);
}

void AGameScene3::OpenCurrentScene() {
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Game3);
}
