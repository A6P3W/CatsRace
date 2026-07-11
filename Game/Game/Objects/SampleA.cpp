#include "Objects/SampleA.h"

#include "Dxlib.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "UMath.h"
REGISTER_ACTOR(ASampleA)
ASampleA::ASampleA(FVector2D location, FRotator rotation) : AActor() {
  SetActorLocation(location);
  int handle =
      ResourceManager::GetInstance().LoadResourceGraph("BaseFile/texture_Checker_64px.png");
  auto* sprite = NewObject<MSpriteComponent>(this);
  sprite->SetRenderSettings(0, RenderSpace::World);
  sprite->SubmitGraph(handle);
  sprite->AttachToComponent(GetRootComponent());
  sprite->RegisterComponent();
}

ASampleA::~ASampleA() {}

void ASampleA::OnUpdate(float DeltaTime) {
  float RotationSpeed = 45.0f;
  float MoveSpeed = 500.0f;
  AddActorRotation(FRotator(RotationSpeed * DeltaTime));        // 毎フレーム回転させる
  AddActorLocalOffset(FVector2D(MoveSpeed * DeltaTime, 0.0f));  // 毎フレーム右に移動させる
}
