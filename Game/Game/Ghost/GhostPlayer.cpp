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

  const int handle = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_1.png");
  auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
  m_SpriteComponent = sprite.get();
  if (handle != -1) {
    m_SpriteComponent->SubmitGraph(handle, FScale(1.0f), 188);
  }
  AddComponent(std::move(sprite));

  auto playback = std::make_unique<MGhostPlaybackComponent>();
  m_PlaybackComponent = playback.get();
  AddComponent(std::move(playback));
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
      0x000000,
      RenderSpace::Screen,
      2,
      180
  );
  renderSystem.SubmitText(
      labelPos, m_UserId, m_UserIdFontHandle, 0xFFFFFF, RenderSpace::Screen, 3, 255
  );
}

void AGhostPlayer::SetUserId(const std::string& UserId) { m_UserId = UserId; }

void AGhostPlayer::SetGhostData(const std::vector<FGhostFrame>& Frames) {
  if (m_PlaybackComponent) {
    m_PlaybackComponent->SetGhostData(Frames);
  }
}
