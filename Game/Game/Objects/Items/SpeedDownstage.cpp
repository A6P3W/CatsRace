#include "SpeedDownstage.h"

#include "Log.h"
#include "MovementComponent.h"
#include "RectangleCollisionComponent.h"
#include "Scenes/Game/Player.h"
#include "SoundComponent.h"
#include "SpriteComponent.h"

REGISTER_ACTOR(ASlowFloor2);

ASlowFloor2::ASlowFloor2(float width, float height, float slowStrength)
    : m_width(width), m_height(height), m_slowStrength(slowStrength) {
  auto* col = NewObject<MRectangleCollisionComponent>(this);
  col->SetSize(m_width, m_height);
  col->SetCollisionType(ECollisionType::Overlap);
  col->SetStatic(true);
  col->AttachToComponent(GetRootComponent());
  col->RegisterComponent();


  m_sound = NewObject<MSoundComponent>(this);
  m_sound->RegisterComponent();

  M_LOG("[SlowFloor2] Spawned. size=({}, {}), strength={}", m_width, m_height, m_slowStrength);
}

void ASlowFloor2::SetSize(float width, float height) {
  m_width = width;
  m_height = height;
}
void ASlowFloor2::BeginOverlap(AActor* OtherActor) {
  if (!OtherActor) return;
  if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
    player->AddSlowSource(this, m_slowStrength);
  }
}

void ASlowFloor2::EndOverlap(AActor* OtherActor) {
  if (!OtherActor) return;
  if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
    player->RemoveSlowSource(this);
  }
}

void ASlowFloor2::OnUpdate(float DeltaTime) {
  std::vector<MMovementComponent*> toRemove;

  for (auto* move : m_overlappingMovements) {
    if (!move || !move->GetOwner() || move->GetOwner()->IsPendingDestroy()) {
      toRemove.push_back(move);
      continue;
    }
  }

  for (auto* dead : toRemove) {
    m_overlappingMovements.erase(dead);
  }
}
