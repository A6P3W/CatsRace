#pragma once

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
  static constexpr float ParticleInterval = 0.02f;
};
