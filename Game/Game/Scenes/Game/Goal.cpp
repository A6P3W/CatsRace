#include "Goal.h"
#include "RectAngleCollisionComponent.h"
#include "SceneManager.h"
#include "Scenes/Game/Player.h"
#include "Log.h"
#include "Scenes/Game/GameSceneBase.h"
REGISTER_ACTOR(AGoalActor)
AGoalActor::AGoalActor(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);

    auto collision = std::make_unique<MRectangleCollisionComponent>();
    collision->SetParentComponent(GetRootComponent());
	collision->SetCollisionType(ECollisionType::Overlap);
    AddComponent(std::move(collision));
}

void AGoalActor::BeginOverlap(AActor* OtherActor)
{
    if(dynamic_cast<APlayer*>(OtherActor)) {
        auto world = GetWorld();
        dynamic_cast<AGameSceneBase*>(world->GetGameMode())->RaceFinish();
    }
}