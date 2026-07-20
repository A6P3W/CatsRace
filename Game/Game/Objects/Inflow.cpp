#include "Objects/Inflow.h"

#include "CircleCollisionComponent.h"
#include "ForceFieldComponent.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"

REGISTER_ACTOR(AInflow)

AInflow::AInflow() {
  if (auto* Range = dynamic_cast<MCircleCollisionComponent*>(GetRangeComponent())) {
    Range->SetRadius(130.0f);
    Range->SetStatic(false);
  }

  if (auto* ForceField = GetForceFieldComponent()) {
    ForceField->SetForceType(EForceFieldType::Point);
    ForceField->SetStrength(-2.5f);
    ForceField->SetActive(true);
  }

  InflowSprite = NewObject<MSpriteComponent>(this);
  if (!InflowSprite) {
    return;
  }

  InflowSprite->SetRenderSettings(0, RenderSpace::World);
  InflowSprite->SubmitGraph(
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/Inflow.png"),
      FScale(1.0f),
      100
  );
  InflowSprite->AttachToComponent(GetRootComponent());
  InflowSprite->RegisterComponent();
}

void AInflow::SetVisualRotationSpeed(float NewSpeed) { VisualRotationSpeed = NewSpeed; }

void AInflow::SetPullStrength(float NewStrength) {
  if (auto* ForceField = GetForceFieldComponent()) {
    ForceField->SetStrength(NewStrength);
  }
}

void AInflow::OnUpdate(float DeltaTime) {
  AForceFieldActor::OnUpdate(DeltaTime);

  if (InflowSprite) {
    InflowSprite->AddWorldRotation(FRotator(VisualRotationSpeed * DeltaTime));
  }
}
