#pragma once
#include <EasyShakeComponent.h>
#include <Pawn.h>

#include <array>
#include <unordered_map>

#include "Actor.h"
#include "SoundComponent.h"
#include "UMath.h"

class MCameraComponent;
class MMovementComponent;
class MSpriteComponent;
class MEasyShakeComponent;
class MCircleCollisionComponent;
class ASlowFloor2;  // ← 前方宣言をここに移動

class APlayer : public APawn {
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
  void AddSlowSource(ASlowFloor2* source, float strength);
  void RemoveSlowSource(ASlowFloor2* source);

 private:
  MCameraComponent* m_camera = nullptr;
  MEasyShakeComponent* m_shake = nullptr;
  MMovementComponent* Movement = nullptr;
  MSpriteComponent* m_sprite = nullptr;
  MCircleCollisionComponent* m_collision = nullptr;
  std::array<int, 5> m_walkAnimHandles{};

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
  float m_driftGauge = 0.0f;
  float m_driftDirection = 0.0f;
  const float MaxDriftGauge = 30.0f;
  const float DriftSteerMultiplier = 1.0f;
  const float DriftMinSpeed = 3.0f;
  const float DriftBoostForce = 20.0f;

  MSoundComponent* m_sound = nullptr;
  int m_engineIdleHandle = -1;
  int m_engineRunHandle = -1;
  float m_spriteTiltAngle = 0.0f;
  const float MaxDriftTiltAngle = 20.0f;
  const float TiltLerpSpeed = 8.0f;
  bool CanMove = false;
  FShakeHandle m_crashshake;

  // ---- スピードダウン管理 ----
  std::unordered_map<ASlowFloor2*, float> m_slowSources;  // ← クラス内に移動

  struct FSkidMark {
    FVector2D Location;
    FRotator Rotation;
    float Alpha;
  };
  std::vector<FSkidMark> m_skidMarks;
  float m_skidTimer = 0.0f;
  static constexpr float SkidInterval = 0.03f;
  static constexpr float SkidFadeSpeed = 0.4f;

  struct FDriftParticle {
    FVector2D Location;
    FVector2D Velocity;
    float Life;
    float MaxLife;
    float Radius;
    bool IsSpark;
  };
  std::vector<FDriftParticle> m_driftParticles;
  float m_particleTimer = 0.0f;
  static constexpr float ParticleInterval = 0.02f;

  void OnDriftPressed();
  void OnDriftReleased();
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
  void UpdateDriftEffect(float DeltaTime);
  void DrawDriftEffect();
  void SpawnSkidMark();
  void SpawnDriftParticles();
  void BeginOverlap(AActor* OtherActor) override;
  void EndOverlap(AActor* OtherActor) override;
};
