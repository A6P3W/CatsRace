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

    APlayer(FVector2D location, FRotator rotation);

	void OnUpdate(float DeltaTime) override;
	void OnPossesed() override;
	void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent);
	
	void SetCanMove(bool canMove) { CanMove = canMove; }
private:
    MCameraComponent* m_camera = nullptr;
    MMovementComponent* m_movement = nullptr;
    MSpriteComponent* m_sprite = nullptr;
    std::array<int, 5> m_walkAnimHandles{};
private:
    float m_accelInput = 0.0f;
    float m_slider = 0.0f;
    
    const float MaxSpeed = 70.0f;  
    const float MaxReverseSpeed = 20.0f; 
    const float AccelForce = 3.5f;   
    const float ReverseForce = 2.0f;   
    const float MaxSteer = 5.5f;
    float m_moveAnimTime = 0.0f;
    int m_walkAnimFrame = 0;

	void OnMove(const FInputActionValue& Value);
	void OnRestartPressed();
	void OnWheel(const FInputActionValue& Value);

	bool CanMove=false;
};
