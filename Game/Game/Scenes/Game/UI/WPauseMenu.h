#pragma once
#include <WidgetBase.h>

#include <functional>
#include <string>

class UITextComponent;
class UIBoxButtonComponent;
class MUIVerticalBoxComponent;

class WPauseMenu : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WPauseMenu);
  WPauseMenu();

  std::function<void()> OnResumePressed;
  std::function<void()> OnRestartPressed;
  std::function<void()> OnTitlePressed;
  std::function<void()> OnLeavePressed;

  void Cancel() override;

 protected:
  void BeginPlay() override;

 private:
  UIBoxButtonComponent* AddMenuButton(const std::string& Label);

  UITextComponent* m_TxtTitle = nullptr;
  MUIVerticalBoxComponent* m_ButtonList = nullptr;

  UIBoxButtonComponent* m_BtnResume = nullptr;

  UIBoxButtonComponent* m_BtnRestart = nullptr;

  UIBoxButtonComponent* m_BtnTitle = nullptr;

  UIBoxButtonComponent* m_BtnLeave = nullptr;
};
