#pragma once

#include "Actor.h"
#include "Ghost/GhostData.h"

#include <vector>

class MGhostPlaybackComponent;
class MSpriteComponent;

class AGhostPlayer : public AActor
{
public:
	DEFINE_ACTOR_CLASS(AGhostPlayer);

	AGhostPlayer(FVector2D location, FRotator rotation);

	void SetGhostData(const std::vector<FGhostFrame>& Frames);
	MGhostPlaybackComponent* GetPlaybackComponent() const { return m_PlaybackComponent; }

private:
	MSpriteComponent* m_SpriteComponent = nullptr;
	MGhostPlaybackComponent* m_PlaybackComponent = nullptr;
};
