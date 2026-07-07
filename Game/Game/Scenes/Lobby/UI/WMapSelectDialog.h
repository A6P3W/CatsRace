#pragma once

#include <WidgetBase.h>

#include <functional>
#include <string>
#include <vector>

class MSpriteComponent;
class UIBoxButtonComponent;
class UITextComponent;

class WMapSelectDialog : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WMapSelectDialog)

  WMapSelectDialog();

  void Cancel() override;

  std::function<void(const std::string&)> OnMapSelected;

 protected:
  void BeginPlay() override;

 private:
  void NotifySelected(const std::string& LevelPath);

  MSpriteComponent* m_BgPanel = nullptr;
  UITextComponent* m_TitleText = nullptr;
  std::vector<UIBoxButtonComponent*> m_MapButtons;
  UIBoxButtonComponent* m_BtnCancel = nullptr;
  UITextComponent* m_TxtCancel = nullptr;
};
