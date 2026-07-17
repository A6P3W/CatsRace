#include "Ghost/GhostPlayer.h"

#include <DxLib.h>

#include "Ghost/GhostPlaybackComponent.h"
#include "RenderSystem.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"

REGISTER_ACTOR(AGhostPlayer)

AGhostPlayer::AGhostPlayer(FVector2D location, FRotator rotation) {
  SetActorLocation(location);
  SetActorRotation(rotation);
  SetActorScale(FScale(0.4f));
  m_UserIdFontHandle = ResourceManager::GetInstance().GetFont(20, 5);

  const int handle =
      ResourceManager::GetInstance().LoadResourceGraph("Resources/images/cat_walk_1.png");
  m_SpriteComponent = NewObject<MSpriteComponent>(this);
  m_SpriteComponent->SetRenderSettings(0, RenderSpace::World);
  if (handle != -1) {
    m_SpriteComponent->SubmitGraph(handle, FScale(1.0f), 188);
  }
  m_SpriteComponent->AttachToComponent(GetRootComponent());
  m_SpriteComponent->RegisterComponent();

  m_PlaybackComponent = NewObject<MGhostPlaybackComponent>(this);
  m_PlaybackComponent->RegisterComponent();
}

void AGhostPlayer::Draw() {
  AActor::Draw();

  if (m_UserId.empty() || m_UserIdFontHandle == -1) {
    return;
  }

  auto& renderSystem = RenderSystem::GetInstance();
  FVector2D labelPos = renderSystem.WorldToScreen(GetActorLocation());
  const int textWidth = GetDrawStringWidthToHandle(
      m_UserId.c_str(), static_cast<int>(m_UserId.length()), m_UserIdFontHandle
  );
  labelPos.X -= textWidth * 0.5f;
  labelPos.Y -= 72.0f;

  renderSystem.SubmitText(
      {labelPos.X + 1.0f, labelPos.Y + 1.0f},
      m_UserId,
      m_UserIdFontHandle,
      FColor{0, 0, 0, 180},
      RenderSpace::Screen,
      2
  );
  renderSystem.SubmitText(
      labelPos, m_UserId, m_UserIdFontHandle, FColor::White, RenderSpace::Screen, 3
  );
}

void AGhostPlayer::SetUserId(const std::string& UserId) { m_UserId = UserId; }

void AGhostPlayer::SetGhostData(const std::vector<FGhostFrame>& Frames) {
  if (m_PlaybackComponent) {
    m_PlaybackComponent->SetGhostData(Frames);
  }
}
