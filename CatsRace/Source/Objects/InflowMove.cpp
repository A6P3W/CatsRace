#include "Objects/InflowMove.h"

REGISTER_ACTOR(AInflowMove)

AInflowMove::AInflowMove() = default;

void AInflowMove::SetMoveSpeed(float NewSpeed) { MoveSpeed = NewSpeed; }

void AInflowMove::OnUpdate(float DeltaTime) {
  AInflow::OnUpdate(DeltaTime);

  if (!bHasAuthority) {
    return;
  }

  AddActorLocalOffset({0.0f, -MoveSpeed * DeltaTime});

  ElapsedTime += DeltaTime;
  if (ElapsedTime >= LifeTime) {
    Destroy();
  }
}
