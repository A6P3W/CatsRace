#pragma once
#include <WidgetBase.h>

#include <functional>
#include <string>

enum class EPostGameResult { PlayAgain, BackToTitle };
using FOnPostGameResult = std::function<void(EPostGameResult)>;

class UITextComponent;
class UIBoxButtonComponent;
class MSpriteComponent;

class WPostGameDialog : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WPostGameDialog);
  WPostGameDialog();

  void SetOnResult(FOnPostGameResult callback);

 protected:
  void BeginPlay() override;

 private:
  FOnPostGameResult m_Callback;

  MSpriteComponent* m_BgPanel = nullptr;
  UITextComponent* m_TitleText = nullptr;

  UIBoxButtonComponent* m_BtnPlayAgain = nullptr;
  UITextComponent* m_TxtPlayAgain = nullptr;

  UIBoxButtonComponent* m_BtnBackToTitle = nullptr;
  UITextComponent* m_TxtBackToTitle = nullptr;
};
