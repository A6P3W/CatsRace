#include "Gameplay/Race/Progress/LapCheckpoint.h"

#include "Gameplay/Race/Player/Player.h"
#include "Log.h"
#include "RectangleCollisionComponent.h"
#include "SpriteComponent.h"
#include "World.h"

namespace {
constexpr float CheckpointCollisionWidth = 1200.0f;
constexpr float CheckpointCollisionHeight = 100.0f;
}  // namespace

REGISTER_ACTOR(ALapCheckpoint);

ALapCheckpoint::ALapCheckpoint(FVector2D location, FRotator rotation) {
  SetActorLocation(location);
  SetActorRotation(rotation);

  m_collision = NewObject<MRectangleCollisionComponent>(this);
  m_collision->SetSize(CheckpointCollisionWidth, CheckpointCollisionHeight);
  m_collision->AttachToComponent(GetRootComponent());
  m_collision->SetCollisionType(ECollisionType::Overlap);
  m_collision->SetStatic(true);
  m_collision->RegisterComponent();

  // ビジュアル（デバッグ表示用）
  // m_sprite = NewObject<MSpriteComponent>(this);
  // m_sprite->SetRenderSettings(0, RenderSpace::World);
  //m_sprite->AttachToComponent(GetRootComponent());
  // m_sprite->SetRelativeLocation({-40.0f, -150.0f});
  // ゴールライン=黄色、通常チェックポイント=水色
  // m_sprite->SubmitBox(
  //     80.0f, 300.0f, m_bIsLapLine ? FColor{255, 255, 0, 180} : FColor{0, 255, 255, 180}, false
  // );
  // m_sprite->RegisterComponent();
}

void ALapCheckpoint::BeginPlay() {
  AActor::BeginPlay();
  // ビジュアルの色をBeginPlay時に確定（コンストラクタ後にSetIsLapLineされる場合のため）
  // if (m_sprite) {
  //  m_sprite->SubmitBox(
  //      80.0f, 300.0f, m_bIsLapLine ? FColor{255, 255, 0, 180} : FColor{0, 255, 255, 180}, false
  // );
  // }
}

void ALapCheckpoint::BeginOverlap(AActor* OtherActor) {
  if (!GetWorld() || !GetWorld()->IsServer()) return;
  if (!OtherActor || OtherActor->IsPendingDestroy()) return;

  auto* player = dynamic_cast<APlayer*>(OtherActor);
  if (!player) return;

  if (m_bIsLapLine) {
    // ゴールライン：全チェックポイント通過済みかチェック
    player->OnLapLineCrossed(m_totalCheckpoints);
  } else {
    // 通常チェックポイント：番号が進んでいる場合のみ更新
    if (player->GetLastPassedCheckpoint() >= m_index) return;
    player->SetLastPassedCheckpoint(m_index);
    M_LOG(Log, "Checkpoint {} passed (conn={})", m_index, player->OwnerConnectionId);
  }
}
