#pragma once

#include "Actor.h"
#include "Ghost/GhostData.h"

#include <string>
#include <vector>

class MGhostPlaybackComponent;
class MSpriteComponent;

class AGhostPlayer : public AActor
{
public:
	DEFINE_ACTOR_CLASS(AGhostPlayer);

	AGhostPlayer(FVector2D location, FRotator rotation);

	void Draw() override;
	void SetUserId(const std::string& UserId);
	void SetGhostData(const std::vector<FGhostFrame>& Frames);
	MGhostPlaybackComponent* GetPlaybackComponent() const { return m_PlaybackComponent; }

private:
	std::string m_UserId;
	int m_UserIdFontHandle = -1;
	MSpriteComponent* m_SpriteComponent = nullptr;
	MGhostPlaybackComponent* m_PlaybackComponent = nullptr;
};
