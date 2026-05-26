#pragma once
#include "Actor.h"
#include <Pawn.h>
#include <Utils/Umath.h>
#include <array>

class MCameraComponent;
class MMovementComponent;
class MSpriteComponent;

class APlayer : public APawn
{
public:
	DEFINE_ACTOR_CLASS(APlayer);
    APlayer(FVector2D location, FRotator rotation);

	void OnUpdate(float DeltaTime) override;
	void OnPossesed() override;
	void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent);
	
private:
    MCameraComponent* m_camera = nullptr;
    MMovementComponent* m_movement = nullptr;
    MSpriteComponent* m_sprite = nullptr;
    std::array<int, 5> m_walkAnimHandles{};
    float m_slider = 0.0f;
    float m_prevSpeed = 0.0f;
    float m_moveAnimTime = 0.0f;
    int m_walkAnimFrame = 0;

	void OnMove(const FInputActionValue& Value);
	void OnRestartPressed();
	void OnWheel(const FInputActionValue& Value);
};
