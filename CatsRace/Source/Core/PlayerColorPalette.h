#pragma once

#include <array>
#include <cstdint>

#include "Color.h"

constexpr uint8_t InvalidPlayerColorIndex = 255;

inline constexpr std::array<FColor, 8> PlayerColorPalette = {
    FColor{255, 90, 90},
    FColor{90, 150, 255},
    FColor{90, 220, 120},
    FColor{255, 215, 80},
    FColor{255, 150, 60},
    FColor{80, 220, 220},
    FColor{190, 100, 255},
    FColor{255, 120, 190},
};
