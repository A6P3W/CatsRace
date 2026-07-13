#include "Scenes/Common/ImGuiHelper.h"

#include <imgui.h>

namespace ImGuiHelper {
void DrawCenteredStatusText(const std::string& Text) {
  const float windowWidth = ImGui::GetWindowSize().x;
  const float textWidth = ImGui::CalcTextSize(Text.c_str()).x;
  ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
  ImGui::TextUnformatted(Text.c_str());
}
}  // namespace ImGuiHelper