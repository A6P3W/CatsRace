#pragma once

#include <vector>

#include "Actor.h"
#include "ActorId.h"

class APlayer;
class MSpriteComponent;

class APlayerDirectionIndicator : public AActor {
 public:
  DEFINE_ACTOR_CLASS(APlayerDirectionIndicator)

  APlayerDirectionIndicator();

  void InitializePlayers(APlayer* LocalPlayer);

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  struct FPlayerIndicatorEntry {
    FActorId TargetActorId = InvalidActorId;
    MSpriteComponent* Sprite = nullptr;
  };

  void AddPlayerEntry(APlayer& Player);
  void UpdateEntry(FPlayerIndicatorEntry& Entry, APlayer& TargetPlayer);
  void RemoveEntrySprite(FPlayerIndicatorEntry& Entry);

  FActorId LocalPlayerActorId = InvalidActorId;
  std::vector<FPlayerIndicatorEntry> Entries;
  bool bInitialized = false;
};
