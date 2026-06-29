#pragma once
#include "Actor.h"
#include "SoundComponent.h"
class SpeedUpItem : public AActor {
 public:
  DEFINE_ACTOR_CLASS(SpeedUpItem);
  SpeedUpItem();
  void BeginOverlap(AActor* OtherActor) override;

 private:
  MSoundComponent* m_sound = nullptr;
};
