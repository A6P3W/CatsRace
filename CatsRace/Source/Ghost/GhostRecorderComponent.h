#pragma once

#include <string>
#include <vector>

#include "ActorComponent.h"
#include "Ghost/GhostData.h"

class MGhostRecorderComponent : public MActorComponent {
 public:
  void StartRecording();
  void StopRecording();

  std::string GetSerializedData() const;
  const std::vector<FGhostFrame>& GetFrames() const { return m_Frames; }
  float GetRecordedSeconds() const { return RecordedSeconds; }
  bool IsPartialGhost() const { return ReachedRecordingLimit; }

 protected:
  void OnUpdate(float DeltaTime) override;

 private:
  void CaptureFrame();

  static constexpr float RecordingInterval = 0.1f;
  static constexpr float MaxRecordingSeconds = 240.0f;

  std::vector<FGhostFrame> m_Frames;
  float m_AccumulatedTime = 0.0f;
  bool m_bRecording = false;
  float RecordedSeconds = 0.0f;
  bool ReachedRecordingLimit = false;
};
