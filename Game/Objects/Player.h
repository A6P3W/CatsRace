#pragma once
#include "Actor.h"
#include <Utils/Umath.h>
class MCameraComponent;
class MMovementComponent;
class APlayer : public AActor
{
public:
	APlayer(FVector2D location ,FRotator rotation);
	void OnUpdate(float DeltaTime) override;
private:
  MCameraComponent* m_camera = nullptr;
  MMovementComponent* m_movement = nullptr;
  float m_slider = 0.0f;
};

