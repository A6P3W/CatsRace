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
	DEFINE_ACTOR_CLASS(APlayer)

    APlayer(FVector2D location, FRotator rotation);
	~APlayer() override;

	void OnUpdate(float DeltaTime) override;
	void OnPossessedBy(APlayerController* NewController) override;
	void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent);
    void ApplyFOVEffect(float targetFOV, float duration, bool showSpeedLines = false);
	void SetCanMove(bool canMove) { CanMove = canMove; }
	void NotifyGoalReached();
    void ApplyFOVEffect(float targetFOV, float duration);

private:
    MCameraComponent* m_camera = nullptr;
	MEasyShakeComponent* m_shake = nullptr;
    MMovementComponent* Movement = nullptr;
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
    const float DriftSpeedDecay = 0.95f;
    float m_moveAnimTime = 5.5f;
    int m_walkAnimFrame = 0;
    float m_fovEffectTimer = 0.0f;
    float m_fovEffectDuration = 0.0f;
    float m_fovTarget = 1.0f;
    float m_fovBase = 1.0f;
    bool m_isDrifting = false;
    bool m_driftKeyPressed = false;
    void OnDriftPressed();
    void OnDriftReleased();
    float m_driftGauge = 0.0f;       // ドリフト中に溜まる量
    float m_driftDirection = 0.0f;   // ドリフト方向(-1 or 1)
    const float MaxDriftGauge = 30.0f;
    const float DriftSteerMultiplier = 1.0f;  // ドリフト中のステア倍率
    const float DriftMinSpeed = 3.0f;         // ドリフト開始に必要な最低速度
    const float DriftBoostForce = 20.0f;       // ブースト力
    void UpdateDrift(float DeltaTime, float speed);
	void UpdateLocalDriftVisual(float DeltaTime, float speed);
	void OnMove(const FInputActionValue& Value);
	void Server_Move(const FVector2D& MoveInput);
	void Server_SetDrift(bool bDriftHeld);
	void Server_NotifyGoal();
	bool IsDriftInputPressed();
	void OnRestartPressed();
	void OnWheel(const FInputActionValue& Value);
    void BeginPlay();
    void DrawSpeedLines(float speed);
	MSoundComponent* m_sound = nullptr;
    int m_engineIdleHandle = -1;   // 停止時
    int m_engineRunHandle = -1;   // 走行時
    float m_spriteTiltAngle = 0.0f;
    const float MaxDriftTiltAngle = 20.0f;   // ドリフト中の最大傾き角度
    const float TiltLerpSpeed = 8.0f;        // 傾きの補間速度
	bool CanMove=false;
	FShakeHandle m_crashshake;
	void BeginOverlap(AActor* OtherActor) override;
    void EndOverlap(AActor* OtherActor) override;
};
