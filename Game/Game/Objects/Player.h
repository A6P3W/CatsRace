#pragma once
#include "Actor.h"
#include <Pawn.h>
#include "UMath.h"
#include <array>
#include <EasyShakeComponent.h>
#include "SoundComponent.h"
class MCameraComponent;
class MMovementComponent;
class MSpriteComponent;
class MEasyShakeComponent;
class MCircleCollisionComponent;

class APlayer : public APawn
{
public:

    APlayer(FVector2D location, FRotator rotation);

	void OnUpdate(float DeltaTime) override;
	void OnPossesed() override;
	void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent);
	
	void SetCanMove(bool canMove) { CanMove = canMove; }
    void ApplyFOVEffect(float targetFOV, float duration);

private:
    MCameraComponent* m_camera = nullptr;
	MEasyShakeComponent* m_shake = nullptr;
    MMovementComponent* m_movement = nullptr;
    MSpriteComponent* m_sprite = nullptr;
    MCircleCollisionComponent* m_collision = nullptr;
    std::array<int, 5> m_walkAnimHandles{};
private:
    float m_accelInput = 0.0f;
    float m_slider = 0.0f;
    bool m_isSpeedUp = false;
    
    const float MaxSpeed = 70.0f;  
    const float MaxReverseSpeed = 20.0f; 
    const float AccelForce = 3.5f;   
    const float ReverseForce = 2.0f;   
    const float MaxSteer = 5.5f;
    float m_moveAnimTime = 0.0f;
    int m_walkAnimFrame = 0;
    float m_fovEffectTimer = 0.0f;
    float m_fovEffectDuration = 0.0f;
    float m_fovTarget = 1.0f;
    float m_fovBase = 1.0f;

	void OnMove(const FInputActionValue& Value);
	void OnRestartPressed();
	void OnWheel(const FInputActionValue& Value);
    void BeginPlay();
    void DrawSpeedLines(float speed);
	MSoundComponent* m_sound = nullptr;
    int m_engineIdleHandle = -1;   // 停止時
    int m_engineRunHandle = -1;   // 走行時

	bool CanMove=false;
	FShakeHandle m_crashshake;
	void BeginOverlap(AActor* OtherActor) override;
    void EndOverlap(AActor* OtherActor) override;
};
