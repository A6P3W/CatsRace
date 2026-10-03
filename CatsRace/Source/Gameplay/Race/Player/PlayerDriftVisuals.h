#pragma once

#include <cstddef>
#include <vector>

#include "Color.h"
#include "UMath.h"

class FPlayerDriftVisuals {
 public:
  FPlayerDriftVisuals();

  void BeginRelease(bool bIsDrifting, float GaugeRatio);
  float GetReleaseGaugeRatio() const { return ReleaseGaugeRatio; }
  void Update(
      float DeltaTime,
      const FVector2D& Location,
      const FRotator& Rotation,
      const FColor& PlayerColor,
      bool bIsDrifting
  );
  void Draw(float GaugeRatio) const;

 private:
  struct FSkidMark {
    FVector2D Location;
    FRotator Rotation;
    float Alpha;
    float Age;
    FColor Color;
  };

  struct FDriftParticle {
    FVector2D Location;
    FVector2D Velocity;
    float Life;
    float MaxLife;
    float Radius;
    bool IsSpark;
  };

  void SpawnSkidMark(const FVector2D& Location, const FRotator& Rotation, const FColor& Color);
  void SpawnDriftParticles(const FVector2D& Location);

  std::vector<FSkidMark> SkidMarks;
  std::vector<FDriftParticle> DriftParticles;
  FVector2D PreviousLocation = FVector2D::ZeroVector();
  float SkidDistance = 0.0f;
  float ParticleTimer = 0.0f;
  float ReleaseGaugeRatio = 0.0f;
  int NextSkidMarkSide = -1;
  int PawPrintHandle = -1;
  bool HasPreviousSkidLocation = false;

  static constexpr float SkidDistanceInterval = 44.0f;
  static constexpr float SkidVisibleDuration = 5.0f;
  static constexpr float SkidFadeDuration = 0.5f;
  static constexpr float SkidMarkScale = 0.5f;
  static constexpr int SkidMarkMaxAlpha = 160;
  static constexpr std::size_t MaxSkidMarkCount = 300;
  static constexpr float TireOffsetX = 6.0f;
  static constexpr float TireOffsetY = 10.0f;
  static constexpr float ParticleInterval = 0.02f;
  static constexpr float SmokeParticleRadius = 6.0f;
  static constexpr float SmokeRadiusGrowthSpeed = 25.0f;
  static constexpr int SmokeParticleCount = 1;
  static constexpr int SparkParticleCount = 3;
  static constexpr float ParticleMinAngleDegrees = 0.0f;
  static constexpr float ParticleMaxAngleDegrees = 360.0f;
  static constexpr float SmokeMinSpeed = 20.0f;
  static constexpr float SmokeMaxSpeed = 80.0f;
  static constexpr float SparkMinSpeed = 100.0f;
  static constexpr float SparkMaxSpeed = 280.0f;
  static constexpr float SmokeMinLifeSeconds = 0.25f;
  static constexpr float SmokeMaxLifeSeconds = 0.55f;
  static constexpr float SparkMinLifeSeconds = 0.05f;
  static constexpr float SparkMaxLifeSeconds = 0.12f;
  static constexpr float ParticleSpawnOffset = 15.0f;
  static constexpr float SkidFadeMaximumAlphaRatio = 0.75f;
  static constexpr float ParticleMaxAlpha = 190.0f;
  static constexpr float SparkLineLengthScale = 0.025f;
  static constexpr float GaugeColorHighThreshold = 0.9f;
  static constexpr float GaugeColorMidThreshold = 0.2f;
};
