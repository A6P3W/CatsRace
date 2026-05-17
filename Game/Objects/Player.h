#pragma once
#include "Actor.h"
#include <Pawn.h>
#include <Utils/Umath.h>
class MCameraComponent;
class MMovementComponent;
class APlayer : public APawn
{
public:
	APlayer();

	void OnUpdate(float DeltaTime) override;
	void OnPossesed() override;

	
private:
	MMovementComponent* m_movement = nullptr;
	float m_slider = 0.0f;

	void OnMove();
};

