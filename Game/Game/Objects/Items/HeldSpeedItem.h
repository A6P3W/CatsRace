#pragma once
#include <TimerHandle.h>  // FTimerHandle の定義が含まれるヘッダー
#include <functional>
#include "Actor.h"
#include "NetworkTypes.h"
#include "SoundComponent.h"

class MCircleCollisionComponent;
class MSpriteComponent;

class AHeldSpeedItem : public AActor {
 public:
  DEFINE_ACTOR_CLASS(AHeldSpeedItem);
  AHeldSpeedItem();
  AHeldSpeedItem(FVector2D location, FRotator rotation);
  void SetOnPickedUp(std::function<void()> callback) { m_onPickedUp = callback; }
  void BeginOverlap(AActor* OtherActor) override;
 private:
  void Multicast_HideAndDestroy();

  // ★ 引数なし、戻り値 void の関数。タイマーマネージャーはこの形式しか受け付けない場合が多いです
  void TriggerDestroy();
  MCircleCollisionComponent* m_collision = nullptr;
  MSpriteComponent* m_sprite = nullptr;
  MSoundComponent* m_sound = nullptr;
  std::function<void()> m_onPickedUp;

  // ★ タイマー管理用のハンドル
  FTimerHandle m_destroyTimerHandle;
};
