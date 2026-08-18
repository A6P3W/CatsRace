#include "Scenes/Lobby/LobbyRoadActor.h"

#include "SpriteComponent.h"

REGISTER_ACTOR(ALobbyRoadActor)

ALobbyRoadActor::ALobbyRoadActor() {
  bReplicates = true;

  SetActorScale(FScale(0.8f));
  SetImagePath("/Game/images/lobby-road.png");

  if (auto* Sprite = dynamic_cast<MSpriteComponent*>(GetRootComponent())) {
    Sprite->SetRenderSettings(-100, RenderSpace::World);
  }
}
