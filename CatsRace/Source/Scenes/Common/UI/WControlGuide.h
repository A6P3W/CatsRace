#pragma once

#include <WidgetBase.h>

#include <string>

class MSpriteComponent;

enum class EControlGuideMode { Game, UI };

class WControlGuide : public AWidgetBase {
 public:
  DEFINE_ACTOR_CLASS(WControlGuide)

  WControlGuide();

  void SetGuideMode(EControlGuideMode Mode);

 protected:
  void BeginPlay() override;
  void OnUpdate(float DeltaTime) override;

 private:
  void UpdateGuideImage();

  MSpriteComponent* GuideSprite = nullptr;
  EControlGuideMode CurrentMode = EControlGuideMode::UI;
  int LastDeviceType = -1;

  int HandleKbGame = -1;
  int HandleKbUI = -1;
  int HandlePadGame = -1;
  int HandlePadUI = -1;

  const std::string PathKbGame = "Resources/images/Guide/KeyboardGameGuide.png";
  const std::string PathKbUI = "Resources/images/Guide/KeyboardUIGuide.png";
  const std::string PathPadGame = "Resources/images/Guide/PadGameGuide.png";
  const std::string PathPadUI = "Resources/images/Guide/PadUIGuide.png";
};