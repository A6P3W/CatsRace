#include "Gameplay/Items/HeldSpeedItem.h"

#include <TimerManager.h>

#include "CircleCollisionComponent.h"
#include "Gameplay/Race/Player/Player.h"
#include "Log.h"
#include "RectangleCollisionComponent.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "World.h"
namespace {
enum : FNetworkRPCId { RPC_MulticastHideAndDestroy = 15 };
constexpr float CollisionWidth = 108.0f;
constexpr float CollisionHeight = 26.0f;
constexpr float SpriteScale = 0.3f;
constexpr float DestroyDelaySeconds = 0.1f;
}  // namespace

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
  m_collision = NewObject<MRectangleCollisionComponent>(this);
  m_collision->SetSize(CollisionWidth, CollisionHeight);
  m_collision->AttachToComponent(GetRootComponent());
  m_collision->SetCollisionType(ECollisionType::Overlap);
  m_collision->SetStatic(true);
  m_collision->RegisterComponent();

  // ビジュアル
  int handle = ResourceManager::GetInstance().LoadResourceGraph("/Game/images/speedup2.png");
  m_sprite = NewObject<MSpriteComponent>(this);
  m_sprite->SetRenderSettings(0, RenderSpace::World);
  m_sprite->AttachToComponent(GetRootComponent());
  m_sprite->SubmitGraph(handle, FScale(SpriteScale), 255);
  m_sprite->RegisterComponent();

  // サウンド
  m_sound = NewObject<MSoundComponent>(this);
  m_sound->RegisterComponent();
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
    m_sound->PlaySE("/Game/images/cat2d.mp3", false);
  }

  M_LOG(Log, "HeldSpeedItem: granted to player (conn={})", player->OwnerConnectionId);
  if (m_onPickedUp) m_onPickedUp();
  Destroy();
}

void AHeldSpeedItem::Multicast_HideAndDestroy() {
  M_LOG(Log, "HeldSpeedItem: Multicast_HideAndDestroy executed.");

  if (GetWorld()) {
    GetWorldTimerManager().SetTimer(
        m_destroyTimerHandle,
        this,
        &AHeldSpeedItem::TriggerDestroy,
        DestroyDelaySeconds,
        false,
        DestroyDelaySeconds
    );
  }
}

// タイマー完了時に呼ばれる破棄関数
void AHeldSpeedItem::TriggerDestroy() {
  M_LOG(Log, "HeldSpeedItem: TriggerDestroy called safely.");

  // サーバー側でこれが呼ばれると、エンジンの基本機能（bReplicates=true）によって
  // クライアント側のアクターも自動的に破棄パケットが飛び、消滅します。
  Destroy();
}
