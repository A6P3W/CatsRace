#pragma once

#include "ForceFieldActor.h"

class MSpriteComponent;

class AInflow : public AForceFieldActor {
 public:
  DEFINE_ACTOR_CLASS(AInflow)
  AInflow();

  void SetVisualRotationSpeed(float NewSpeed);
  void SetPullStrength(float NewStrength);

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  MSpriteComponent* InflowSprite = nullptr;
  float VisualRotationSpeed = -160.0f;
};
