#include "SpeedUpItem.h"

#include <SpriteComponent.h>

#include "CircleCollisionComponent.h"
#include "Gameplay/Race/Player/Player.h"
#include "Log.h"
#include "MovementComponent.h"
#include "RectangleCollisionComponent.h"
#include "SoundComponent.h"

namespace {
constexpr float CollisionWidth = 195.0f;
constexpr float CollisionHeight = 98.0f;
constexpr float SpriteScale = 0.1f;
constexpr float SpeedBoostForce = 25.0f;
constexpr float BoostFOV = 0.7f;
constexpr float BoostDurationSeconds = 2.0f;
}  // namespace

REGISTER_ACTOR(SpeedUpItem);
SpeedUpItem::SpeedUpItem() {
  auto* collision = NewObject<MRectangleCollisionComponent>(this);
  collision->SetSize(CollisionWidth, CollisionHeight);
  collision->AttachToComponent(GetRootComponent());
  collision->SetCollisionType(ECollisionType::Overlap);
  collision->RegisterComponent();

  int handle = ResourceManager::GetInstance().LoadResourceGraph("/Game/images/speedfloa.png");
  auto* sprite = NewObject<MSpriteComponent>(this);
  sprite->SetRenderSettings(0, RenderSpace::World);
  sprite->SubmitGraph(handle, FScale(SpriteScale), 255);
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

  player->GetComponents<MMovementComponent>()[0]->AddLocalForce({0, -SpeedBoostForce});
  player->ApplyFOVEffect(BoostFOV, BoostDurationSeconds, true);
  if (m_sound) m_sound->PlaySE("/Game/images/cat2d.mp3", false);
  M_LOG(Log, "Speed Up!");
}
