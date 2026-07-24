#pragma once

#include "ForceFieldActor.h"

class ADirectionalForceFieldActor : public AForceFieldActor {
 public:
  DEFINE_ACTOR_CLASS(ADirectionalForceFieldActor)
  ADirectionalForceFieldActor();

  void SetForceStrength(float NewStrength);
  float GetForceStrength() const;

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  float ForceStrength = 1.5f;
};
