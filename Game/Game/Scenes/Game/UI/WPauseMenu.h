#pragma once
#include <WidgetBase.h>

#include <functional>

class UITextComponent;
class UIBoxButtonComponent;

class WPauseMenu : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WPauseMenu);
  WPauseMenu();

  std::function<void()> OnResumePressed;
  std::function<void()> OnRestartPressed;
  std::function<void()> OnTitlePressed;

  void Cancel() override;

 protected:
  void BeginPlay() override;

 private:
  UITextComponent* m_TxtTitle = nullptr;

  UIBoxButtonComponent* m_BtnResume = nullptr;
  UITextComponent* m_TxtResume = nullptr;

  UIBoxButtonComponent* m_BtnRestart = nullptr;
  UITextComponent* m_TxtRestart = nullptr;

  UIBoxButtonComponent* m_BtnTitle = nullptr;
  UITextComponent* m_TxtTitleBtn = nullptr;
};
