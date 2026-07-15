#include "SpeedDown.h"

#include "Log.h"
#include "MovementComponent.h"
#include "RectangleCollisionComponent.h"
#include "Scenes/Game/Player.h"
#include "SoundComponent.h"
#include "SpriteComponent.h"

REGISTER_ACTOR(ASlowFloor);

ASlowFloor::ASlowFloor(float width, float height, float slowStrength)
    : m_width(width), m_height(height), m_slowStrength(slowStrength) {
  auto* col = NewObject<MRectangleCollisionComponent>(this);
  col->SetSize(m_width, m_height);
  col->SetCollisionType(ECollisionType::Overlap);
  col->SetStatic(true);
  col->AttachToComponent(GetRootComponent());
  col->RegisterComponent();

  auto* sprite = NewObject<MSpriteComponent>(this);
  sprite->SetRenderSettings(10, RenderSpace::World);
  sprite->SubmitBox(m_width, m_height, 0x4488FF, 1, 100);
  sprite->SetRelativeLocation({-m_width * 0.5f, -m_height * 0.5f});
  sprite->AttachToComponent(GetRootComponent());
  sprite->RegisterComponent();

  // サウンドコンポーネントはコンストラクタで初期化する
  m_sound = NewObject<MSoundComponent>(this);
  m_sound->RegisterComponent();

  M_LOG("[SlowFloor] Spawned. size=({}, {}), strength={}", m_width, m_height, m_slowStrength);
}

void ASlowFloor::SetSize(float width, float height) {
  m_width = width;
  m_height = height;
}

// BeginOverlapは1つだけ
void ASlowFloor::BeginOverlap(AActor* OtherActor) {
  if (!OtherActor) return;

  // プレイヤーならFOVを狭めてSEを鳴らす
  if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
    player->ApplyFOVEffect(1.4f, 3.0f, false);
    if (m_sound) m_sound->PlaySE("images/cat19.mp3", false);
  }

  auto movements = OtherActor->GetComponents<MMovementComponent>();
  for (auto* move : movements) {
    m_overlappingMovements.insert(move);
    M_LOG("[SlowFloor] BeginOverlap: {}", OtherActor->GetActorClassName());
  }
}

void ASlowFloor::EndOverlap(AActor* OtherActor) {
  if (!OtherActor) return;
  auto movements = OtherActor->GetComponents<MMovementComponent>();
  for (auto* move : movements) {
    m_overlappingMovements.erase(move);
    M_LOG("[SlowFloor] EndOverlap: {}", OtherActor->GetActorClassName());
  }
}

void ASlowFloor::OnUpdate(float DeltaTime) {
  std::vector<MMovementComponent*> toRemove;

  for (auto* move : m_overlappingMovements) {
    if (!move || !move->GetOwner() || move->GetOwner()->IsPendingDestroy()) {
      toRemove.push_back(move);
      continue;
    }

    float decayPerFrame = std::pow(m_slowStrength, DeltaTime * 60.0f);
    FVector2D v = move->GetVelocity();
    move->SetWorldVelocity(v * decayPerFrame);
  }

  for (auto* dead : toRemove) {
    m_overlappingMovements.erase(dead);
  }
}
