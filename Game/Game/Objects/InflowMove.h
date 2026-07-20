#pragma once

#include "Objects/Inflow.h"

class AInflowMove : public AInflow {
 public:
  DEFINE_ACTOR_CLASS(AInflowMove)
  AInflowMove();

  void SetMoveSpeed(float NewSpeed);

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  static constexpr float LifeTime = 45.0f;

  float MoveSpeed = 250.0f;
  float ElapsedTime = 0.0f;
};
