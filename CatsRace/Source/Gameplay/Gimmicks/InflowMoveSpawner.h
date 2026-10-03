#pragma once

#include "Actor.h"

class MSpriteComponent;

class AInflowMoveSpawner : public AActor {
 public:
  DEFINE_ACTOR_CLASS(AInflowMoveSpawner)
  AInflowMoveSpawner();

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  void SpawnInflowMove();

  static constexpr float SpawnInterval = 5.0f;
  static constexpr float MinSpawnAngle = -15.0f;
  static constexpr float MaxSpawnAngle = 15.0f;

  MSpriteComponent* DirectionSprite = nullptr;
  float SpawnElapsedTime = 0.0f;
};
