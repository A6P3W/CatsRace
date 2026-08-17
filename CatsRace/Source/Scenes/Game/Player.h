#pragma once
#include <EasyShakeComponent.h>
#include <Pawn.h>

#include <array>
#include <string>
#include <unordered_map>

#include "Actor.h"
#include "Core/PlayerColorPalette.h"
#include "SoundComponent.h"
#include "UMath.h"

class MCameraComponent;
class MNetMovementComponent;
class MSpriteComponent;
class MEasyShakeComponent;
class MRectangleCollisionComponent;
class ASlowFloor2;  // ← 前方宣言をここに移動

class APlayer : public APawn {
 public:
  DEFINE_ACTOR_CLASS(APlayer)

  APlayer(FVector2D location, FRotator rotation);
  ~APlayer() override;

  void OnUpdate(float DeltaTime) override;
  void Draw() override;
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
  void SetRotateCamera(bool bInRotateCamera) { bRotateCamera = bInRotateCamera; }
  bool IsRotateCamera() const { return bRotateCamera; }
  void SetPlayerName(const std::string& PlayerName);
  const std::string& GetPlayerName() const { return m_PlayerName; }
  void SetPlayerColorIndex(uint8_t InColorIndex);
  uint8_t GetPlayerColorIndex() const { return PlayerColorIndex; }
  void SetSpeedMultiplier(float InMultiplier);
  float GetSpeedMultiplier() const { return SpeedMultiplier; }

  static constexpr float MinSpeedMultiplier = 0.8f;
  static constexpr float MaxSpeedMultiplier = 2.0f;
  static constexpr float DefaultSpeedMultiplier = 1.0f;

 private:
  MCameraComponent* m_camera = nullptr;
  MEasyShakeComponent* m_shake = nullptr;
  MNetMovementComponent* Movement = nullptr;
  MSpriteComponent* m_sprite = nullptr;
  uint8_t PlayerColorIndex = InvalidPlayerColorIndex;
  FColor CurrentPlayerColor = FColor::White;
  MRectangleCollisionComponent* m_collision = nullptr;
  std::array<int, 5> m_walkAnimHandles{};

  bool bRotateCamera = true;
  float m_accelInput = 0.0f;
  float m_slider = 0.0f;
  bool m_isSpeedUp = false;
  float m_lapLineCooldown = 0.0f;
  float SpeedMultiplier = DefaultSpeedMultiplier;
  static constexpr float BaseMaxSpeed = 10.0f;
  static constexpr float BaseMaxReverseSpeed = 20.0f;
  static constexpr float AccelForce = 3.5f;
  static constexpr float ReverseForce = 2.0f;
  static constexpr float BaseMaxSteer = 2.5f;
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
  std::string m_PlayerName;
  int m_PlayerNameFontHandle = -1;
  int m_engineIdleHandle = -1;
  int m_engineRunHandle = -1;
  float m_spriteTiltAngle = 0.0f;
  const float MaxDriftTiltAngle = 20.0f;
  const float TiltLerpSpeed = 8.0f;
  bool CanMove = false;
  bool m_hasHeldItem = false;
  void Server_UseHeldItem();
  void ApplyPlayerColor();
  void OnRepPlayerColorIndex(uint8_t OldColorIndex);
  void ApplyHeldItemEffect();
  FShakeHandle m_crashshake;
  // ---- スピードダウン管理 ----
  std::unordered_map<ASlowFloor2*, float> m_slowSources;  // ← クラス内に移動
  int m_currentLap = 0;                                   // 完了した周回数（0始まり）
  int m_lastPassedCheckpoint = -1;                        // 最後に通過したチェックポイント番号
  static constexpr int TotalLaps = 2;                     // 総周回数
  struct FSkidMark {
    FVector2D Location;
    FRotator Rotation;
    float Alpha;
    float Age;
    FColor Color;
  };
  std::vector<FSkidMark> m_skidMarks;
  float SkidDistance = 0.0f;
  bool bHasPreviousSkidLocation = false;
  float SkidReleaseTimer = 0.0f;
  float SkidReleaseGaugeRatio = 0.0f;
  int NextSkidMarkSide = -1;
  int PawPrintHandle = -1;
  static constexpr float SkidDistanceInterval = 44.0f;
  static constexpr float SkidVisibleDuration = 5.0f;
  static constexpr float SkidFadeDuration = 0.5f;
  static constexpr float SkidReleaseDuration = 0.2f;
  static constexpr float SkidMarkScale = 0.5f;
  static constexpr int SkidMarkMaxAlpha = 160;

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
  void Server_SyncDriftState(bool bDrifting, float driftDirection, float driftGaugeRatio);
  void Server_NotifyGoal();
  bool IsDriftInputPressed();
  void OnRestartPressed();
  void OnWheel(const FInputActionValue& Value);
  void BeginPlay();
  void DrawSpeedLines(float speed);
  void UpdateDriftEffect(float DeltaTime);
  void DrawDriftEffect();
  void BeginSkidReleaseTrail();
  void SpawnSkidMark(const FVector2D& Location);
  void SpawnDriftParticles();
  void BeginOverlap(AActor* OtherActor) override;
  void EndOverlap(AActor* OtherActor) override;
};
