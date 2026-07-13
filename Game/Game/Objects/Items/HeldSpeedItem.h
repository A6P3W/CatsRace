#pragma once
#include <TimerHandle.h>

#include <functional>

#include "Actor.h"
#include "NetworkTypes.h"
#include "RectangleCollisionComponent.h"
#include "SoundComponent.h"

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
  void TriggerDestroy();

  MRectangleCollisionComponent* m_collision = nullptr;  
  MSpriteComponent* m_sprite = nullptr;
  MSoundComponent* m_sound = nullptr;
  std::function<void()> m_onPickedUp;
  FTimerHandle m_destroyTimerHandle;
};
