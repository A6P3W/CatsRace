#pragma once

#include <WidgetBase.h>

#include <functional>

class MSpriteComponent;
class UIBoxButtonComponent;
class UITextComponent;

class WLeaveLobbyConfirmDialog : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WLeaveLobbyConfirmDialog)

  WLeaveLobbyConfirmDialog();

  void Cancel() override;

  std::function<void(bool)> OnResult;

 protected:
  void BeginPlay() override;

 private:
  void NotifyResult(bool bLeaveLobby);

  MSpriteComponent* m_BgPanel = nullptr;
  UITextComponent* m_MessageText = nullptr;
  UIBoxButtonComponent* m_YesButton = nullptr;
  UIBoxButtonComponent* m_NoButton = nullptr;
};
