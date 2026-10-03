#include "Objects/InflowMoveSpawner.h"

#include <random>

#include "Objects/InflowMove.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "World.h"

namespace {
float GetRandomSpawnAngle(float MinAngle, float MaxAngle) {
  static thread_local std::mt19937 RandomEngine{std::random_device{}()};
  std::uniform_real_distribution<float> Distribution{MinAngle, MaxAngle};
  return Distribution(RandomEngine);
}
}  // namespace

REGISTER_ACTOR(AInflowMoveSpawner)

AInflowMoveSpawner::AInflowMoveSpawner() {
  bReplicates = true;

  DirectionSprite = NewObject<MSpriteComponent>(this);
  if (!DirectionSprite) {
    return;
  }

  DirectionSprite->SetRenderSettings(0, RenderSpace::World);
  DirectionSprite->SubmitGraph(
      ResourceManager::GetInstance().LoadResourceGraph("/Engine/Images/arrow-up.png")
  );
  DirectionSprite->AttachToComponent(GetRootComponent());
  DirectionSprite->RegisterComponent();
}

void AInflowMoveSpawner::OnUpdate(float DeltaTime) {
  AActor::OnUpdate(DeltaTime);

  if (!bHasAuthority) {
    return;
  }

  SpawnElapsedTime += DeltaTime;
  while (SpawnElapsedTime >= SpawnInterval) {
    SpawnElapsedTime -= SpawnInterval;
    SpawnInflowMove();
  }
}

void AInflowMoveSpawner::SpawnInflowMove() {
  World* CurrentWorld = GetWorld();
  if (!CurrentWorld) {
    return;
  }

  const FRotator SpawnRotation =
      GetActorRotation() + FRotator(GetRandomSpawnAngle(MinSpawnAngle, MaxSpawnAngle));
  CurrentWorld->SpawnActor<AInflowMove>(GetActorLocation(), SpawnRotation);
}
