#include "Scenes/Lobby/MapPreviewActor.h"

#include "ActorManager.h"
#include "Core/MapData.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"
#include "World.h"

REGISTER_ACTOR(AMapPreviewActor)

AMapPreviewActor::AMapPreviewActor() {
  PreviewSprite = NewObject<MSpriteComponent>(this);
  SetRootComponent(PreviewSprite);
  if (PreviewSprite) {
    PreviewSprite->SetRenderSettings(0, RenderSpace::World);
    PreviewSprite->RegisterComponent();
  }
}

AMapPreviewActor::~AMapPreviewActor() {
  if (LobbyPlayerState && MapChangedHandle != 0) {
    LobbyPlayerState->RemoveOnSelectedMapChanged(MapChangedHandle);
  }
}

void AMapPreviewActor::BeginPlay() {
  AActor::BeginPlay();
  RegisterSelectedMapChangedCallback();
}

void AMapPreviewActor::OnUpdate(float DeltaTime) {
  AActor::OnUpdate(DeltaTime);
  if (!LobbyPlayerState) {
    RegisterSelectedMapChangedCallback();
  }
  UpdateTransition(DeltaTime);
}

void AMapPreviewActor::HandleSelectedMapChanged(const std::string& LevelPath) {
  if (LevelPath.empty()) {
    return;
  }
  PendingLevelPath = LevelPath;
  TransitionState = ETransitionState::ScalingOut;
}

void AMapPreviewActor::ChangePreviewImage() {
  if (!PreviewSprite) {
    return;
  }
  const FMapInfo* mapInfo = FindMapInfo(PendingLevelPath);
  if (!mapInfo) {
    return;
  }
  const int graphHandle =
      ResourceManager::GetInstance().LoadResourceGraph(mapInfo->LobbyPreviewPath);
  if (graphHandle != -1) {
    PreviewSprite->SubmitGraph(graphHandle);
  }
}

void AMapPreviewActor::UpdateTransition(float DeltaTime) {
  if (!PreviewSprite || TransitionState == ETransitionState::Idle) {
    return;
  }
  const float speed = TransitionDuration > 0.0f ? 1.0f / TransitionDuration : 1.0f;
  switch (TransitionState) {
    case ETransitionState::ScalingOut:
      CurrentScale -= speed * DeltaTime;
      if (CurrentScale <= 0.0f) {
        CurrentScale = 0.0f;
        ChangePreviewImage();
        TransitionState = ETransitionState::ScalingIn;
      }
      break;
    case ETransitionState::ScalingIn:
      CurrentScale += speed * DeltaTime;
      if (CurrentScale >= 1.0f) {
        CurrentScale = 1.0f;
        TransitionState = ETransitionState::Idle;
      }
      break;
    case ETransitionState::Idle:
      return;
  }
  PreviewSprite->SetRelativeScale(FScale(CurrentScale));
}

void AMapPreviewActor::RegisterSelectedMapChangedCallback() {
  if (LobbyPlayerState || !GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }
  for (const auto& actor : GetWorld()->GetActorManager()->GetAllActors()) {
    auto* playerState = dynamic_cast<ALobbyPlayerState*>(actor.get());
    if (!playerState || playerState->IsPendingDestroy() || playerState->OwnerConnectionId != 0) {
      continue;
    }
    LobbyPlayerState = playerState;
    MapChangedHandle =
        LobbyPlayerState->AddOnSelectedMapChanged([this](const std::string& LevelPath) {
          HandleSelectedMapChanged(LevelPath);
        });
    PendingLevelPath = LobbyPlayerState->GetSelectedLevelPath();
    ChangePreviewImage();
    return;
  }
}
