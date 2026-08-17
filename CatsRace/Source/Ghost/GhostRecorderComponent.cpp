#include "Ghost/GhostRecorderComponent.h"

#include <algorithm>
#include <cmath>

#include "Actor.h"

void MGhostRecorderComponent::StartRecording() {
  m_Frames.clear();
  m_AccumulatedTime = 0.0f;
  RecordedSeconds = 0.0f;
  ReachedRecordingLimit = false;
  m_bRecording = true;
  CaptureFrame();
}

void MGhostRecorderComponent::StopRecording() { m_bRecording = false; }

std::string MGhostRecorderComponent::GetSerializedData() const {
  return GhostDataSerializer::Serialize(m_Frames);
}

void MGhostRecorderComponent::OnUpdate(float DeltaTime) {
  if (!m_bRecording) {
    return;
  }

  const float RemainingSeconds = (std::max)(0.0f, MaxRecordingSeconds - RecordedSeconds);
  const float RecordedDelta = (std::min)(DeltaTime, RemainingSeconds);
  RecordedSeconds += RecordedDelta;
  m_AccumulatedTime += RecordedDelta;
  while (m_AccumulatedTime >= RecordingInterval) {
    m_AccumulatedTime -= RecordingInterval;
    CaptureFrame();
  }
  if (RecordedSeconds >= MaxRecordingSeconds) {
    ReachedRecordingLimit = true;
    m_bRecording = false;
  }
}

void MGhostRecorderComponent::CaptureFrame() {
  auto* owner = GetOwner();
  if (!owner) {
    return;
  }

  const auto location = owner->GetActorLocation();
  const auto rotation = owner->GetActorRotation();

  FGhostFrame frame;
  frame.X = static_cast<int>(std::lround(location.X));
  frame.Y = static_cast<int>(std::lround(location.Y));
  frame.Rot = static_cast<int>(std::lround(rotation.Rotation * 10.0f));
  m_Frames.push_back(frame);
}
