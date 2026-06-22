#pragma once

#include "ActorComponent.h"
#include "Ghost/GhostData.h"

#include <vector>

class MGhostPlaybackComponent : public MActorComponent
{
public:
	void SetGhostData(const std::vector<FGhostFrame>& Frames);
	void UpdatePlayback(float RaceTime);

	const std::vector<FGhostFrame>& GetFrames() const { return m_Frames; }

private:
	static constexpr float PlaybackInterval = 0.1f;

	std::vector<FGhostFrame> m_Frames;
};
