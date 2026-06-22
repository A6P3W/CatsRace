#pragma once

#include "ActorComponent.h"
#include "Ghost/GhostData.h"

#include <string>
#include <vector>

class MGhostRecorderComponent : public MActorComponent
{
public:
	void StartRecording();
	void StopRecording();

	std::string GetSerializedData() const;
	const std::vector<FGhostFrame>& GetFrames() const { return m_Frames; }

protected:
	void OnUpdate(float DeltaTime) override;

private:
	void CaptureFrame();

	static constexpr float RecordingInterval = 0.1f;

	std::vector<FGhostFrame> m_Frames;
	float m_AccumulatedTime = 0.0f;
	bool m_bRecording = false;
};
