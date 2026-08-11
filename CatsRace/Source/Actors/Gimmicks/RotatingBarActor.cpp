#include "Actors/Gimmicks/RotatingBarActor.h"

#include <cmath>

#include "RectangleCollisionComponent.h"
#include "SpriteComponent.h"
#include "TimerManager.h"

REGISTER_ACTOR(ARotatingBarActor)

ARotatingBarActor::ARotatingBarActor() {
  bReplicates = true;
  RegisterReplicatedProperty(&RotationSpeed);
  RegisterReplicatedProperty(&bClockwise);
  RegisterReplicatedProperty(&bRotating);

  BarCollision = NewObject<MRectangleCollisionComponent>(this);
  if (BarCollision) {
    SetRootComponent(BarCollision);
    BarCollision->SetSize(BarWidth, BarHeight);
    BarCollision->SetCollisionType(ECollisionType::Block);
    BarCollision->SetStatic(false);
    BarCollision->RegisterComponent();
  }

  BarSprite = NewObject<MSpriteComponent>(this);
  if (BarSprite) {
    BarSprite->SetRenderSettings(0, RenderSpace::World);
    UpdateBoxAppearance();
    BarSprite->AttachToComponent(BarCollision ? BarCollision : GetRootComponent());
    BarSprite->RegisterComponent();
  }
}

void ARotatingBarActor::SetRotationSpeed(float NewRotationSpeed) {
  RotationSpeed = std::abs(NewRotationSpeed);
}

float ARotatingBarActor::GetRotationSpeed() const { return RotationSpeed; }

void ARotatingBarActor::SetClockwise(bool bNewClockwise) { bClockwise = bNewClockwise; }

bool ARotatingBarActor::IsClockwise() const { return bClockwise; }

void ARotatingBarActor::SetRotating(bool bNewRotating) { bRotating = bNewRotating; }

bool ARotatingBarActor::IsRotating() const { return bRotating; }

void ARotatingBarActor::SetBarSize(float NewWidth, float NewHeight) {
  BarWidth = NewWidth > 0.0f ? NewWidth : 0.0f;
  BarHeight = NewHeight > 0.0f ? NewHeight : 0.0f;
  if (BarCollision) {
    BarCollision->SetSize(BarWidth, BarHeight);
  }
  UpdateBoxAppearance();
}

float ARotatingBarActor::GetBarWidth() const { return BarWidth; }

float ARotatingBarActor::GetBarHeight() const { return BarHeight; }

void ARotatingBarActor::BeginPlay() {
  AActor::BeginPlay();

  if (!bHasAuthority) {
    return;
  }

  GetWorldTimerManager().SetTimer(
      DirectionReverseTimer,
      this,
      &ARotatingBarActor::ReverseRotationDirection,
      DirectionReverseInterval,
      true
  );
}

void ARotatingBarActor::OnUpdate(float DeltaTime) {
  AActor::OnUpdate(DeltaTime);

  if (!bHasAuthority || !bRotating || DeltaTime <= 0.0f) {
    return;
  }

  const float DirectionScale = bClockwise ? 1.0f : -1.0f;
  AddActorRotation(FRotator(RotationSpeed * DeltaTime * DirectionScale));
}

void ARotatingBarActor::ReverseRotationDirection() { bClockwise = !bClockwise; }

void ARotatingBarActor::UpdateBoxAppearance() {
  if (BarSprite) {
    BarSprite->SubmitBox(BarWidth, BarHeight, FColor{255, 120, 60}, true);
  }
}
