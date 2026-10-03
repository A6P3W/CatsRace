#pragma once

#include "Actor.h"
#include "TimerHandle.h"

class MRectangleCollisionComponent;
class MSpriteComponent;

class ARotatingBarActor : public AActor {
 public:
  DEFINE_ACTOR_CLASS(ARotatingBarActor)
  ARotatingBarActor();

  void SetRotationSpeed(float NewRotationSpeed);
  float GetRotationSpeed() const;

  void SetClockwise(bool bNewClockwise);
  bool IsClockwise() const;

  void SetRotating(bool bNewRotating);
  bool IsRotating() const;

  void SetBarSize(float NewWidth, float NewHeight);
  float GetBarWidth() const;
  float GetBarHeight() const;

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void ReverseRotationDirection();
  void UpdateBoxAppearance();

  static constexpr float DirectionReverseInterval = 10.0f;

  float RotationSpeed = 75.0f;
  bool bClockwise = true;
  bool bRotating = true;
  float BarWidth = 300.0f;
  float BarHeight = 10.0f;

  MSpriteComponent* BarSprite = nullptr;
  MRectangleCollisionComponent* BarCollision = nullptr;
  FTimerHandle DirectionReverseTimer;
};
