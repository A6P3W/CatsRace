#pragma once
#include "Actor.h"
#include "SoundComponent.h"
#include "ResourceManager.h" 
class MRectangleCollisionComponent;
class MSpriteComponent;
class MSoundComponent;
class SpeedUpItem : public AActor {
 public:
  DEFINE_ACTOR_CLASS(SpeedUpItem);
  SpeedUpItem();
  void BeginOverlap(AActor* OtherActor) override;

 private:
  MRectangleCollisionComponent* m_collision = nullptr;
  MSoundComponent* m_sound = nullptr;
  MSpriteComponent* m_sprite = nullptr;
};
