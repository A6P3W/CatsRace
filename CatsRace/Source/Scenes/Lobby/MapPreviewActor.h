#pragma once

#include <string>

#include "Actor.h"
#include "Scenes/Lobby/LobbyPlayerState.h"

class MSpriteComponent;

class AMapPreviewActor : public AActor {
 public:
  DEFINE_ACTOR_CLASS(AMapPreviewActor)

  AMapPreviewActor();
  ~AMapPreviewActor() override;

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  enum class ETransitionState { Idle, ScalingOut, ScalingIn };

  void HandleSelectedMapChanged(const std::string& LevelPath);
  void ChangePreviewImage();
  void UpdateTransition(float DeltaTime);
  void RegisterSelectedMapChangedCallback();

  MSpriteComponent* PreviewSprite = nullptr;
  ALobbyPlayerState* LobbyPlayerState = nullptr;
  ETransitionState TransitionState = ETransitionState::Idle;
  std::string PendingLevelPath;
  float CurrentScale = 1.0f;
  float TransitionDuration = 0.15f;
  ALobbyPlayerState::FCallbackHandle MapChangedHandle = 0;
};
