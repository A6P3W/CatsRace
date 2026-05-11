#pragma once
#include "Actor.h"
#include <Utils/Umath.h>
class MCameraComponent;
class APlayer : public AActor
{
public:
	APlayer(FVector2D location ,FRotator rotation);
	void OnUpdate(float DeltaTime) override;
private:
  MCameraComponent* m_camera = nullptr;
};

