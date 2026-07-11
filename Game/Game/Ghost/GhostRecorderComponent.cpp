#include "Ghost/GhostRecorderComponent.h"

#include <cmath>

#include "Actor.h"

void MGhostRecorderComponent::StartRecording() {
  m_Frames.clear();
  m_AccumulatedTime = 0.0f;
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

  m_AccumulatedTime += DeltaTime;
  while (m_AccumulatedTime >= RecordingInterval) {
    m_AccumulatedTime -= RecordingInterval;
    CaptureFrame();
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
