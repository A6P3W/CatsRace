#pragma once
#include "Actor.h"
#include <Utils/Umath.h>
class AMap : public AActor
{
public:
	DEFINE_ACTOR_CLASS(AMap);
	AMap(FVector2D location, FRotator rotation);
};

