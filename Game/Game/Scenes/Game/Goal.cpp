#include "Goal.h"

#include "Log.h"
#include "RectAngleCollisionComponent.h"
#include "Scenes/Game/Player.h"
#include "World.h"

REGISTER_ACTOR(AGoalActor)

AGoalActor::AGoalActor(FVector2D location, FRotator rotation) {
  SetActorLocation(location);

  auto collision = std::make_unique<MRectangleCollisionComponent>();
  collision->SetParentComponent(GetRootComponent());
  collision->SetCollisionType(ECollisionType::Overlap);
  AddComponent(std::move(collision));
}

void AGoalActor::BeginOverlap(AActor* OtherActor) {
  auto* player = dynamic_cast<APlayer*>(OtherActor);
  if (!player) {
    return;
  }

  if (GetWorld()->IsServer() || player->bIsLocallyControlled) {
    player->NotifyGoalReached();
  }
}