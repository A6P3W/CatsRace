#pragma once

#include <cstdint>

enum class ERacePhase : uint8_t {
  WaitingForStart,
  Countdown,
  Running,
  ResultPending,
  TravelingToResult,
};
