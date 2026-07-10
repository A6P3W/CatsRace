#include "Objects/Items/HeldSpeedItem.h"

#include <TimerManager.h>  // タイマーマネージャーのヘッダー

#include "CircleCollisionComponent.h"
#include "Log.h"
#include "Scenes/Game/Player.h"
#include "SpriteComponent.h"
#include "World.h"

namespace {
enum : FNetworkRPCId { RPC_MulticastHideAndDestroy = 15 };
}

REGISTER_ACTOR(AHeldSpeedItem);

AHeldSpeedItem::AHeldSpeedItem() {
  bReplicates = true;

  RegisterRPC(
      RPC_MulticastHideAndDestroy,
      ENetRPCType::Multicast,
      this,
      &AHeldSpeedItem::Multicast_HideAndDestroy
  );

  // 当たり判定（Overlap）
  auto collision = std::make_unique<MCircleCollisionComponent>(80.0f);
  m_collision = collision.get();
  m_collision->SetParentComponent(GetRootComponent());
  m_collision->SetCollisionType(ECollisionType::Overlap);
  m_collision->SetStatic(true);
  AddComponent(std::move(collision));

  // ビジュアル
  auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
  m_sprite = sprite.get();
  m_sprite->SetParentComponent(GetRootComponent());
  m_sprite->SubmitCircle(80.0f, 0xFF4444, true, 200);
  AddComponent(std::move(sprite));

  // サウンド
  auto sound = std::make_unique<MSoundComponent>();
  m_sound = sound.get();
  AddComponent(std::move(sound));
}

AHeldSpeedItem::AHeldSpeedItem(FVector2D location, FRotator rotation) : AHeldSpeedItem() {
  SetActorLocation(location);
  SetActorRotation(rotation);
}

void AHeldSpeedItem::BeginOverlap(AActor* OtherActor) {
  // 1. サーバーのみで判定処理（クライアント側は完全スルー）
  if (!GetWorld() || !bHasAuthority) {
    return;
  }

  auto* player = dynamic_cast<APlayer*>(OtherActor);
  if (!player || player->HasHeldItem()) {
    return;
  }

  player->GrantHeldItem();

  if (m_sound) {
    m_sound->PlaySE("Resources/images/cat2d.mp3", false);
  }

  M_LOG("HeldSpeedItem: granted to player (conn={})", player->OwnerConnectionId);
  if (m_onPickedUp) m_onPickedUp();
  Destroy();
}

void AHeldSpeedItem::Multicast_HideAndDestroy() {
  M_LOG("HeldSpeedItem: Multicast_HideAndDestroy executed.");


  if (GetWorld()) {
    GetWorldTimerManager().SetTimer(
        m_destroyTimerHandle, this, &AHeldSpeedItem::TriggerDestroy, 0.1f, false, 0.1f
    );
  }
}

// タイマー完了時に呼ばれる破棄関数
void AHeldSpeedItem::TriggerDestroy() {
  M_LOG("HeldSpeedItem: TriggerDestroy called safely.");

  // サーバー側でこれが呼ばれると、エンジンの基本機能（bReplicates=true）によって
  // クライアント側のアクターも自動的に破棄パケットが飛び、消滅します。
  Destroy();
}
