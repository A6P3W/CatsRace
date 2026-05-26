#pragma once
#include "Actor.h"

class AGoalActor : public AActor
{
public:
	DEFINE_ACTOR_CLASS(AGoalActor);
    AGoalActor(FVector2D location, FRotator rotation);

    // 他のアクター（プレイヤー）が重なった時に呼ばれる
    void BeginOverlap(AActor* OtherActor) override;
};