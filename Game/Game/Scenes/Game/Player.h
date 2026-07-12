#pragma once
#include <EasyShakeComponent.h>
#include <Pawn.h>

#include <array>
#include <unordered_map>

#include "Actor.h"
#include "SoundComponent.h"
#include "UMath.h"

class MCameraComponent;
class MNetMovementComponent;
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
  bool HasHeldItem() const { return m_hasHeldItem; }
  void GrantHeldItem();
  void UseHeldItem();
  int GetCurrentLap() const { return m_currentLap; }
  int GetLastPassedCheckpoint() const { return m_lastPassedCheckpoint; }
  void SetLastPassedCheckpoint(int index) { m_lastPassedCheckpoint = index; }
  void OnLapLineCrossed(int totalCheckpoints);
 private:
  MCameraComponent* m_camera = nullptr;
  MEasyShakeComponent* m_shake = nullptr;
  MNetMovementComponent* Movement = nullptr;
  MSpriteComponent* m_sprite = nullptr;
  MCircleCollisionComponent* m_collision = nullptr;
  std::array<int, 5> m_walkAnimHandles{};

  float m_accelInput = 0.0f;
  float m_slider = 0.0f;
  bool m_isSpeedUp = false;
  float m_lapLineCooldown = 0.0f;
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
  FVector2D m_prevLocation = FVector2D::ZeroVector();
  MSoundComponent* m_sound = nullptr;
  int m_engineIdleHandle = -1;
  int m_engineRunHandle = -1;
  float m_spriteTiltAngle = 0.0f;
  const float MaxDriftTiltAngle = 20.0f;
  const float TiltLerpSpeed = 8.0f;
  bool CanMove = false;
  bool m_hasHeldItem = false;
  void Server_UseHeldItem();
  void ApplyHeldItemEffect();
  FShakeHandle m_crashshake;
  // ---- スピードダウン管理 ----
  std::unordered_map<ASlowFloor2*, float> m_slowSources;  // ← クラス内に移動
  int m_currentLap = 0;                                   // 完了した周回数（0始まり）
  int m_lastPassedCheckpoint = -1;                        // 最後に通過したチェックポイント番号
  static constexpr int TotalLaps = 3;                     // 総周回数
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
  void Multicast_UpdateLap(int newLap);
  void OnDriftPressed();
  void OnDriftReleased();
  void UpdateDrift(float DeltaTime, float speed);
  void UpdateLocalDriftVisual(float DeltaTime, float speed);
  void OnMove(const FInputActionValue& Value);
  void Server_SetDrift(bool bDriftHeld);
  void Server_SyncDriftState(bool bDrifting, float driftDirection);
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
