#pragma once
#include "Actor.h"
class SpeedUpItem:public AActor
{
public:
	DEFINE_ACTOR_CLASS(SpeedUpItem);
	SpeedUpItem();
	void BeginOverlap(AActor* OtherActor) override;
};

