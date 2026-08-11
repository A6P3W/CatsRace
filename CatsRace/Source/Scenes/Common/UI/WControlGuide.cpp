#include "Scenes/Common/UI/WControlGuide.h"

#include <InputManager.h>
#include <ResourceManager.h>
#include <SpriteComponent.h>
#include <UIWidgetComponent.h>

REGISTER_ACTOR(WControlGuide)

WControlGuide::WControlGuide() {
  SetUpdateableAnytime(true);

  auto* RootWidget = NewObject<MUIWidgetComponent>(this);
  SetRootComponent(RootWidget);
  RootWidget->SetAnchor(EUIAnchor::BottomRight);
  RootWidget->SetPivot({1.0f, 1.0f});
  RootWidget->SetWidgetSize({250.0f, 250.0f});
  RootWidget->SetAnchoredPosition({-20.0f, -20.0f});
  RootWidget->RegisterComponent();

  GuideSprite = NewObject<MSpriteComponent>(this);
  GuideSprite->SetRenderSettings(100, RenderSpace::Screen);
  GuideSprite->AttachToComponent(RootWidget);
  GuideSprite->SetRelativeLocation({0.0f, 0.0f});
  GuideSprite->RegisterComponent();
}

void WControlGuide::BeginPlay() {
  AWidgetBase::BeginPlay();

  auto& ResourceManager = ResourceManager::GetInstance();
  HandleKbGame = ResourceManager.LoadResourceGraph(PathKbGame);
  HandleKbUI = ResourceManager.LoadResourceGraph(PathKbUI);
  HandlePadGame = ResourceManager.LoadResourceGraph(PathPadGame);
  HandlePadUI = ResourceManager.LoadResourceGraph(PathPadUI);

  const auto DeviceType = InputManager::GetInstance().GetLastInputDevice();
  LastDeviceType = DeviceType == EInputDeviceType::Gamepad ? 1 : 0;
  UpdateGuideImage();
}

void WControlGuide::SetGuideMode(EControlGuideMode Mode) {
  if (CurrentMode == Mode) {
    return;
  }

  CurrentMode = Mode;
  UpdateGuideImage();
}

void WControlGuide::OnUpdate(float DeltaTime) {
  AWidgetBase::OnUpdate(DeltaTime);

  const auto Device = InputManager::GetInstance().GetLastInputDevice();
  const int DeviceType = Device == EInputDeviceType::Gamepad ? 1 : 0;
  if (LastDeviceType == DeviceType) {
    return;
  }

  LastDeviceType = DeviceType;
  UpdateGuideImage();
}

void WControlGuide::UpdateGuideImage() {
  if (!GuideSprite) {
    return;
  }

  const bool bIsGamepad = LastDeviceType == 1;
  int TargetHandle = -1;
  if (CurrentMode == EControlGuideMode::Game) {
    TargetHandle = bIsGamepad ? HandlePadGame : HandleKbGame;
  } else {
    TargetHandle = bIsGamepad ? HandlePadUI : HandleKbUI;
  }

  GuideSprite->SetVisibility(TargetHandle != -1);
  if (TargetHandle != -1) {
    GuideSprite->SubmitGraph(TargetHandle, FScale(0.5f), 200);
  }
}