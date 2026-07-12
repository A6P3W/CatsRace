#include "SpeedUpItem.h"

#include <SpriteComponent.h>
#include "RectangleCollisionComponent.h" 
#include "CircleCollisionComponent.h"
#include "Log.h"
#include "MovementComponent.h"
#include "Scenes/Game/Player.h"
#include "SoundComponent.h"
REGISTER_ACTOR(SpeedUpItem);
SpeedUpItem::SpeedUpItem() {
  auto collision = std::make_unique<MRectangleCollisionComponent>(1950.0f, 980.0f);
  m_collision = collision.get();
  m_collision->SetParentComponent(GetRootComponent());
  m_collision->SetCollisionType(ECollisionType::Overlap);
  m_collision->SetStatic(true);
  AddComponent(std::move(collision));
  int handle = ResourceManager::GetInstance().LoadResourceGraph("Resources/images/speedfloa.png");
  auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
  m_sprite = sprite.get();  
  sprite->SubmitGraph(handle, FScale(1.0f), 255);
  sprite->SetParentComponent(GetRootComponent());
  AddComponent(std::move(sprite));

  auto sound = std::make_unique<MSoundComponent>();
  m_sound = sound.get();
  AddComponent(std::move(sound));
}

void SpeedUpItem::BeginOverlap(AActor* OtherActor) {
  if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
    player->GetComponents<MMovementComponent>()[0]->AddLocalForce({0, -25.0f});
    player->ApplyFOVEffect(0.7f, 2.0f, true);
    if (m_sound) m_sound->PlaySE("images/cat2d.mp3", false);  // お好みのSEパスに変更
    M_LOG("Speed Up!");
  }
}
