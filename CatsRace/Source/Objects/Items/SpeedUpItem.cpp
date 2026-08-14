#include "SpeedUpItem.h"

#include <SpriteComponent.h>

#include "CircleCollisionComponent.h"
#include "Log.h"
#include "MovementComponent.h"
#include "RectangleCollisionComponent.h"
#include "Scenes/Game/Player.h"
#include "SoundComponent.h"
REGISTER_ACTOR(SpeedUpItem);
SpeedUpItem::SpeedUpItem() {
  auto* collision = NewObject<MRectangleCollisionComponent>(this);
  collision->SetSize(195.0f, 98.0f);
  collision->AttachToComponent(GetRootComponent());
  collision->SetCollisionType(ECollisionType::Overlap);
  collision->RegisterComponent();

  int handle = ResourceManager::GetInstance().LoadResourceGraph("/Game/images/speedfloa.png");
  auto* sprite = NewObject<MSpriteComponent>(this);
  sprite->SetRenderSettings(0, RenderSpace::World);
  sprite->SubmitGraph(handle, FScale(0.1f), 255);
  sprite->AttachToComponent(GetRootComponent());
  sprite->RegisterComponent();

  m_sound = NewObject<MSoundComponent>(this);
  m_sound->RegisterComponent();
}

void SpeedUpItem::BeginOverlap(AActor* OtherActor) {
  if (!GetWorld() || !GetWorld()->IsServer()) {
    return;
  }

  auto* player = dynamic_cast<APlayer*>(OtherActor);
  if (!player) {
    return;
  }

  player->GetComponents<MMovementComponent>()[0]->AddLocalForce({0, -25.0f});
  player->ApplyFOVEffect(0.7f, 2.0f, true);
  if (m_sound) m_sound->PlaySE("/Game/images/cat2d.mp3", false);
  M_LOG(Log, "Speed Up!");
}
