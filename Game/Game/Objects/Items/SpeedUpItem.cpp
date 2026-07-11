#include "SpeedUpItem.h"

#include <SpriteComponent.h>

#include "CircleCollisionComponent.h"
#include "Log.h"
#include "MovementComponent.h"
#include "Scenes/Game/Player.h"
#include "SoundComponent.h"
REGISTER_ACTOR(SpeedUpItem);
SpeedUpItem::SpeedUpItem() {
  auto* collision = NewObject<MCircleCollisionComponent>(this);
  collision->SetRadius(100.0f);
  collision->AttachToComponent(GetRootComponent());
  collision->SetCollisionType(ECollisionType::Overlap);
  collision->RegisterComponent();

  auto* sprite = NewObject<MSpriteComponent>(this);
  sprite->SetRenderSettings(0, RenderSpace::World);
  sprite->SubmitCircle(100.0f, 0x00FF00, 1, 128);
  sprite->AttachToComponent(GetRootComponent());
  sprite->RegisterComponent();

  m_sound = NewObject<MSoundComponent>(this);
  m_sound->RegisterComponent();
}

void SpeedUpItem::BeginOverlap(AActor* OtherActor) {
  if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
    player->GetComponents<MMovementComponent>()[0]->AddLocalForce({0, -25.0f});
    player->ApplyFOVEffect(0.7f, 2.0f, true);
    if (m_sound) m_sound->PlaySE("images/cat2d.mp3", false);  // お好みのSEパスに変更
    M_LOG("Speed Up!");
  }
}