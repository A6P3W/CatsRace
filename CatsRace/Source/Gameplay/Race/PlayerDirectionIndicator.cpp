#include "PlayerDirectionIndicator.h"

#include <algorithm>
#include <cmath>

#include "ActorManager.h"
#include "Core/PlayerColorPalette.h"
#include "Geometry2D.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "Scenes/Game/Player.h"
#include "SpriteComponent.h"
#include "World.h"

namespace {
constexpr int IndicatorPriority = 100;
constexpr float IndicatorMargin = 64.0f;
constexpr float DirectionEpsilonSquared = 1e-6f;
constexpr float ImageRotationOffset = 0.0f;
}  // namespace

REGISTER_ACTOR(APlayerDirectionIndicator)

APlayerDirectionIndicator::APlayerDirectionIndicator() { bReplicates = false; }

void APlayerDirectionIndicator::InitializePlayers(APlayer* LocalPlayer) {
  if (bInitialized || !LocalPlayer || !GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }

  FActorManager* ActorManager = GetWorld()->GetActorManager();
  LocalPlayerActorId = LocalPlayer->GetActorId();
  if (LocalPlayerActorId == InvalidActorId) {
    return;
  }

  for (const auto& ActorPtr : ActorManager->GetAllActors()) {
    APlayer* Player = ActorPtr ? dynamic_cast<APlayer*>(ActorPtr.get()) : nullptr;
    if (!Player || Player->IsPendingDestroy() || Player == LocalPlayer) {
      continue;
    }

    AddPlayerEntry(*Player);
  }

  bInitialized = true;
}

void APlayerDirectionIndicator::OnUpdate(float DeltaTime) {
  AActor::OnUpdate(DeltaTime);

  if (!bInitialized || !GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }

  FActorManager* ActorManager = GetWorld()->GetActorManager();
  APlayer* LocalPlayer = dynamic_cast<APlayer*>(ActorManager->FindActorById(LocalPlayerActorId));
  if (!LocalPlayer) {
    for (FPlayerIndicatorEntry& Entry : Entries) {
      if (Entry.Sprite) {
        Entry.Sprite->SetVisibility(false);
      }
    }
    return;
  }

  for (auto Iterator = Entries.begin(); Iterator != Entries.end();) {
    if (!Iterator->Sprite) {
      Iterator = Entries.erase(Iterator);
      continue;
    }

    APlayer* TargetPlayer =
        dynamic_cast<APlayer*>(ActorManager->FindActorById(Iterator->TargetActorId));
    if (!TargetPlayer) {
      RemoveEntrySprite(*Iterator);
      Iterator = Entries.erase(Iterator);
      continue;
    }

    UpdateEntry(*Iterator, *TargetPlayer);
    ++Iterator;
  }
}

void APlayerDirectionIndicator::AddPlayerEntry(APlayer& Player) {
  const FActorId ActorId = Player.GetActorId();
  if (ActorId == InvalidActorId) {
    return;
  }

  MSpriteComponent* Sprite = NewObject<MSpriteComponent>(this);
  if (!Sprite) {
    return;
  }

  const int ImageHandle = ResourceManager::GetInstance().LoadResourceGraph(
      "/Game/images/cat_black_white_circle_100.png"
  );
  Sprite->SetRenderSettings(IndicatorPriority, RenderSpace::Screen);
  Sprite->SubmitGraph(ImageHandle);
  Sprite->SetVisibility(false);
  if (Player.GetPlayerColorIndex() < PlayerColorPalette.size()) {
    Sprite->SetTint(PlayerColorPalette[Player.GetPlayerColorIndex()]);
  }
  Sprite->RegisterComponent();

  Entries.push_back({ActorId, Sprite});
}

void APlayerDirectionIndicator::UpdateEntry(FPlayerIndicatorEntry& Entry, APlayer& TargetPlayer) {
  if (!Entry.Sprite) {
    return;
  }

  RenderSystem& Renderer = RenderSystem::GetInstance();
  const FRect2D ScreenRect = Renderer.GetScreenRect();
  const FVector2D ScreenCenter = ScreenRect.GetCenter();
  const FVector2D TargetScreen = Renderer.WorldToScreen(TargetPlayer.GetActorLocation());
  if (Renderer.IsScreenPointVisible(TargetScreen)) {
    Entry.Sprite->SetVisibility(false);
    return;
  }

  const FVector2D Direction = TargetScreen - ScreenCenter;
  if (Direction.SizeSquared() < DirectionEpsilonSquared) {
    Entry.Sprite->SetVisibility(false);
    return;
  }

  const FRect2D IndicatorRect = ScreenRect.Inset(IndicatorMargin);
  FVector2D Intersection;
  if (!UGeometry2D::RayIntersectRect(ScreenCenter, Direction, IndicatorRect, Intersection)) {
    Entry.Sprite->SetVisibility(false);
    return;
  }

  const float AngleDegrees = UMath::RadToDeg(std::atan2(Direction.Y, Direction.X));
  Entry.Sprite->SetRelativeLocation(Intersection);
  Entry.Sprite->SetWorldRotation(FRotator(AngleDegrees + ImageRotationOffset));
  if (TargetPlayer.GetPlayerColorIndex() < PlayerColorPalette.size()) {
    Entry.Sprite->SetTint(PlayerColorPalette[TargetPlayer.GetPlayerColorIndex()]);
    FScale Scale = FScale(std::clamp(500 / Direction.Size(), 0.5f, 1.0f));
    Entry.Sprite->SetWorldScale(Scale);
  }
  Entry.Sprite->SetVisibility(true);
}

void APlayerDirectionIndicator::RemoveEntrySprite(FPlayerIndicatorEntry& Entry) {
  if (!Entry.Sprite) {
    return;
  }

  Entry.Sprite->SetVisibility(false);
  Entry.Sprite->DestroyComponent();
  Entry.Sprite = nullptr;
}
