#include "Actors/Gimmicks/DirectionalForceFieldActor.h"

#include "ForceFieldComponent.h"

REGISTER_ACTOR(ADirectionalForceFieldActor)

ADirectionalForceFieldActor::ADirectionalForceFieldActor() {
  RegisterReplicatedProperty(&ForceStrength);
  MForceFieldComponent* ForceField = GetForceFieldComponent();
  if (!ForceField) {
    return;
  }

  ForceField->SetForceType(EForceFieldType::Directional);
  ForceField->SetDirection({1.0f, 0.0f});
  ForceField->SetStrength(ForceStrength);
  ForceField->SetActive(true);
}

void ADirectionalForceFieldActor::SetForceStrength(float NewStrength) {
  ForceStrength = NewStrength;

  MForceFieldComponent* ForceField = GetForceFieldComponent();
  if (ForceField) {
    ForceField->SetStrength(ForceStrength);
  }
}

float ADirectionalForceFieldActor::GetForceStrength() const { return ForceStrength; }

void ADirectionalForceFieldActor::OnUpdate(float DeltaTime) {
  AForceFieldActor::OnUpdate(DeltaTime);

  MForceFieldComponent* ForceField = GetForceFieldComponent();
  if (ForceField) {
    ForceField->SetDirection(FVector2D{1.0f, 0.0f}.RotateVector(GetActorRotation()));
  }
}
