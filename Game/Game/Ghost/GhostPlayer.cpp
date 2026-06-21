#include "Ghost/GhostPlayer.h"

#include "Ghost/GhostPlaybackComponent.h"
#include "ResourceManager.h"
#include "SpriteComponent.h"

REGISTER_ACTOR(AGhostPlayer)

AGhostPlayer::AGhostPlayer(FVector2D location, FRotator rotation)
{
	SetActorLocation(location);
	SetActorRotation(rotation);
	SetActorScale(FScale(0.4f));

	const int handle = ResourceManager::GetInstance().LoadResourceGraph("images/cat_walk_1.png");
	auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
	m_SpriteComponent = sprite.get();
	if (handle != -1) {
		m_SpriteComponent->SubmitGraph(handle, FScale(1.0f), 128);
	}
	AddComponent(std::move(sprite));

	auto playback = std::make_unique<MGhostPlaybackComponent>();
	m_PlaybackComponent = playback.get();
	AddComponent(std::move(playback));
}

void AGhostPlayer::SetGhostData(const std::vector<FGhostFrame>& Frames)
{
	if (m_PlaybackComponent) {
		m_PlaybackComponent->SetGhostData(Frames);
	}
}
