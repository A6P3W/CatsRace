#include "PlayerDriftVisuals.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "UMath.h"

FPlayerDriftVisuals::FPlayerDriftVisuals() {
  PawPrintHandle = ResourceManager::GetInstance().LoadResourceGraph("/Game/images/paw-print.png");
}

void FPlayerDriftVisuals::BeginRelease(bool bIsDrifting, float GaugeRatio) {
  if (!bIsDrifting) {
    return;
  }
  ReleaseGaugeRatio = GaugeRatio;
}

void FPlayerDriftVisuals::Update(
    float DeltaTime,
    const FVector2D& Location,
    const FRotator& Rotation,
    const FColor& PlayerColor,
    bool bIsDrifting
) {
  if (!HasPreviousSkidLocation) {
    PreviousLocation = Location;
    HasPreviousSkidLocation = true;
  } else {
    const FVector2D MovedVector = Location - PreviousLocation;
    float RemainingDistance = MovedVector.Size();
    if (RemainingDistance > 0.0f) {
      const float Interval = bIsDrifting ? SkidDistanceInterval * 0.5f : SkidDistanceInterval;
      if (SkidDistance >= Interval) {
        SkidDistance = std::fmod(SkidDistance, Interval);
      }
      FVector2D SegmentStart = PreviousLocation;
      const FVector2D Direction = MovedVector / RemainingDistance;
      while (SkidDistance + RemainingDistance >= Interval) {
        const float DistanceToMark = Interval - SkidDistance;
        const FVector2D MarkLocation = SegmentStart + Direction * DistanceToMark;
        SpawnSkidMark(MarkLocation, Rotation, PlayerColor);
        SegmentStart = MarkLocation;
        RemainingDistance -= DistanceToMark;
        SkidDistance = 0.0f;
      }
      SkidDistance += RemainingDistance;
    } else {
      SkidDistance = 0.0f;
    }
  }
  PreviousLocation = Location;

  for (auto& Mark : SkidMarks) {
    Mark.Age += DeltaTime;
    if (Mark.Age > SkidVisibleDuration) {
      Mark.Alpha = std::clamp(
          1.0f - (Mark.Age - SkidVisibleDuration) / SkidFadeDuration,
          0.0f,
          SkidFadeMaximumAlphaRatio
      );
    }
  }
  SkidMarks.erase(
      std::remove_if(
          SkidMarks.begin(),
          SkidMarks.end(),
          [](const FSkidMark& Mark) { return Mark.Alpha <= 0.0f; }
      ),
      SkidMarks.end()
  );

  if (bIsDrifting) {
    ParticleTimer += DeltaTime;
    if (ParticleTimer >= ParticleInterval) {
      ParticleTimer = 0.0f;
      SpawnDriftParticles(Location);
    }
  } else {
    ParticleTimer = 0.0f;
  }

  for (auto& Particle : DriftParticles) {
    Particle.Location = Particle.Location + Particle.Velocity * DeltaTime;
    Particle.Life -= DeltaTime;
    if (!Particle.IsSpark) {
      Particle.Radius += SmokeRadiusGrowthSpeed * DeltaTime;
    }
  }
  DriftParticles.erase(
      std::remove_if(
          DriftParticles.begin(),
          DriftParticles.end(),
          [](const FDriftParticle& Particle) { return Particle.Life <= 0.0f; }
      ),
      DriftParticles.end()
  );
}

void FPlayerDriftVisuals::SpawnSkidMark(
    const FVector2D& Location, const FRotator& Rotation, const FColor& Color
) {
  if (SkidMarks.size() >= MaxSkidMarkCount) {
    SkidMarks.erase(SkidMarks.begin());
  }

  const FVector2D Offset =
      FVector2D(TireOffsetX * NextSkidMarkSide, TireOffsetY).RotateVector(Rotation);
  SkidMarks.push_back({Location + Offset, Rotation, 1.0f, 0.0f, Color});
  NextSkidMarkSide *= -1;
}

void FPlayerDriftVisuals::SpawnDriftParticles(const FVector2D& Location) {
  static std::mt19937 Rng{std::random_device{}()};
  std::uniform_real_distribution<float> DistAngle(ParticleMinAngleDegrees, ParticleMaxAngleDegrees);
  std::uniform_real_distribution<float> DistSmoke(SmokeMinSpeed, SmokeMaxSpeed);
  std::uniform_real_distribution<float> DistSpark(SparkMinSpeed, SparkMaxSpeed);
  std::uniform_real_distribution<float> DistLife(SmokeMinLifeSeconds, SmokeMaxLifeSeconds);
  std::uniform_real_distribution<float> DistSparkLife(SparkMinLifeSeconds, SparkMaxLifeSeconds);
  std::uniform_real_distribution<float> DistOffset(-ParticleSpawnOffset, ParticleSpawnOffset);

  for (int Index = 0; Index < SmokeParticleCount; ++Index) {
    const float Angle = UMath::DegToRad(DistAngle(Rng));
    const float Speed = DistSmoke(Rng);
    const float Life = DistLife(Rng);
    DriftParticles.push_back({
        {Location.X + DistOffset(Rng), Location.Y + DistOffset(Rng)},
        {std::cos(Angle) * Speed, std::sin(Angle) * Speed},
        Life,
        Life,
        SmokeParticleRadius,
        false,
    });
  }

  for (int Index = 0; Index < SparkParticleCount; ++Index) {
    const float Angle = UMath::DegToRad(DistAngle(Rng));
    const float Speed = DistSpark(Rng);
    const float Life = DistSparkLife(Rng);
    DriftParticles.push_back({
        {Location.X + DistOffset(Rng), Location.Y + DistOffset(Rng)},
        {std::cos(Angle) * Speed, std::sin(Angle) * Speed},
        Life,
        Life,
        0.0f,
        true,
    });
  }
}

void FPlayerDriftVisuals::Draw(float GaugeRatio) const {
  auto& Renderer = RenderSystem::GetInstance();
  for (const auto& Mark : SkidMarks) {
    if (PawPrintHandle == -1) {
      continue;
    }
    Renderer.SubmitGraph(
        Mark.Location,
        PawPrintHandle,
        FScale(SkidMarkScale),
        Mark.Rotation,
        RenderSpace::World,
        2,
        static_cast<int>(Mark.Alpha * SkidMarkMaxAlpha),
        Mark.Color
    );
  }

  for (const auto& Particle : DriftParticles) {
    const int Alpha = static_cast<int>(Particle.Life / Particle.MaxLife * ParticleMaxAlpha);
    if (Particle.IsSpark) {
      FColor SparkColor{255, 204, 0, static_cast<uint8_t>(Alpha)};
      if (GaugeRatio > GaugeColorHighThreshold) {
        SparkColor = FColor{255, 68, 68, static_cast<uint8_t>(Alpha)};
      } else if (GaugeRatio > GaugeColorMidThreshold) {
        SparkColor = FColor{68, 204, 255, static_cast<uint8_t>(Alpha)};
      }
      const FVector2D Tip = Particle.Location + Particle.Velocity * SparkLineLengthScale;
      Renderer.SubmitLine(Particle.Location, Tip, SparkColor, RenderSpace::World, 3);
    } else {
      Renderer.SubmitCircle(
          Particle.Location,
          Particle.Radius,
          FColor{187, 187, 187, static_cast<uint8_t>(Alpha)},
          true,
          RenderSpace::World,
          3
      );
    }
  }
}
