#include "Ghost/GhostPlaybackComponent.h"

#include "Actor.h"

#include <algorithm>
#include <cmath>

namespace
{
	float Lerp(float A, float B, float Alpha)
	{
		return A + (B - A) * Alpha;
	}
}

void MGhostPlaybackComponent::SetGhostData(const std::vector<FGhostFrame>& Frames)
{
	m_Frames = Frames;

	if (!m_Frames.empty()) {
		UpdatePlayback(0.0f);
	}
}

void MGhostPlaybackComponent::UpdatePlayback(float RaceTime)
{
	auto* owner = GetOwner();
	if (!owner || m_Frames.empty()) {
		return;
	}

	if (RaceTime <= 0.0f || m_Frames.size() == 1) {
		const auto& frame = m_Frames.front();
		owner->SetActorLocation({ static_cast<float>(frame.X), static_cast<float>(frame.Y) });
		owner->SetActorRotation(FRotator(static_cast<float>(frame.Rot) / 10.0f));
		return;
	}

	const float framePosition = RaceTime / PlaybackInterval;
	const size_t frameIndex = static_cast<size_t>(std::floor(framePosition));

	if (frameIndex >= m_Frames.size() - 1) {
		const auto& frame = m_Frames.back();
		owner->SetActorLocation({ static_cast<float>(frame.X), static_cast<float>(frame.Y) });
		owner->SetActorRotation(FRotator(static_cast<float>(frame.Rot) / 10.0f));
		return;
	}

	const auto& current = m_Frames[frameIndex];
	const auto& next = m_Frames[frameIndex + 1];
	const float alpha = std::clamp(framePosition - static_cast<float>(frameIndex), 0.0f, 1.0f);

	const float x = Lerp(static_cast<float>(current.X), static_cast<float>(next.X), alpha);
	const float y = Lerp(static_cast<float>(current.Y), static_cast<float>(next.Y), alpha);
	const float rot = Lerp(static_cast<float>(current.Rot), static_cast<float>(next.Rot), alpha) / 10.0f;

	owner->SetActorLocation({ x, y });
	owner->SetActorRotation(FRotator(rot));
}
