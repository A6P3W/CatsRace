#include "Goal.h"
#include "RectAngleCollisionComponent.h"
#include "SceneManager.h"
#include "Objects/Player.h"
#include "Log.h"
#include "GameScene01.h"
AGoalActor::AGoalActor(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);

    auto collision = std::make_unique<MRectangleCollisionComponent>();
    collision->SetParentComponent(GetRootComponent());
    AddComponent(std::move(collision));
}

void AGoalActor::BeginOverlap(AActor* OtherActor)
{
    if(dynamic_cast<APlayer*>(OtherActor)) {
        auto world = GetWorld();
        dynamic_cast<AGameScene01*>(world->GetGameMode())->RaceFinish();
    }
}