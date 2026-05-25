#pragma once
#include "Actor.h"
class SpeedUpItem:public AActor
{
public:
	SpeedUpItem();
	void BeginOverlap(AActor* OtherActor) override;
};

