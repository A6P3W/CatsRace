#include "WNameInputDialog.h"

#include <DxLib.h>
#include <SpriteComponent.h>
#include <UIBoxButton.h>
#include <UITextComponent.h>

namespace {
std::string KeepAlphanumericOnly(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  for (char c : str) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
      result.push_back(c);
    }
  }
  return result;
}
}  // namespace

WNameInputDialog::WNameInputDialog() {
  // Background Panel (ZOrder: 0)
  m_BgPanel = NewObject<MSpriteComponent>(this);
  m_BgPanel->SetRenderSettings(0, RenderSpace::Screen);
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 175.0f});
  m_BgPanel->SubmitBox(600, 350, FColor{15, 15, 20, 220}, true);
  m_BgPanel->RegisterComponent();

  // Title Text (ZOrder: 1)
  m_TitleText = NewObject<UITextComponent>(this);
  m_TitleText->SetText("登録名を入力してください");
  m_TitleText->SetColor(FColor{255, 255, 255});
  m_TitleText->SetFontSize(24);
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -100.0f});
  m_TitleText->RegisterComponent();

  // Input Box Background (ZOrder: 1)
  m_InputBoxBg = NewObject<MSpriteComponent>(this);
  m_InputBoxBg->SetRenderSettings(1, RenderSpace::Screen);
  m_InputBoxBg->SetRelativeLocation({960.0f - 200.0f, 540.0f - 35.0f});
  m_InputBoxBg->SubmitBox(400, 50, FColor{30, 35, 45, 255}, true);
  m_InputBoxBg->RegisterComponent();

  // Input Box Text Component (ZOrder: 2)
  m_InputText = NewObject<UITextComponent>(this);
  m_InputText->SetText("");
  m_InputText->SetColor(FColor{255, 255, 255});
  m_InputText->SetFontSize(24);
  m_InputText->SetPivot({0.5f, 0});
  m_InputText->SetAnchoredPosition({0.0f, 0.0f});  // Left margin inside box
  m_InputText->RegisterComponent();

  // Submit Button (ZOrder: 1)
  m_BtnSubmit = NewObject<UIBoxButtonComponent>(this);
  m_BtnSubmit->SetSize(200.0f, 45.0f);
  m_BtnSubmit->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnSubmit->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnSubmit->SetPivot({0.5f, 0.5f});
  m_BtnSubmit->SetAnchoredPosition({-120.0f, 80.0f});
  m_BtnSubmit->RegisterComponent();

  m_TxtSubmit = NewObject<UITextComponent>(this);
  m_TxtSubmit->SetText("送信 (ENTER)");
  m_TxtSubmit->SetColor(FColor{255, 255, 255});
  m_TxtSubmit->SetFontSize(18);
  m_TxtSubmit->AttachToComponent(m_BtnSubmit);
  m_TxtSubmit->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtSubmit->SetPivot({0.5f, 0.5f});
  m_TxtSubmit->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtSubmit->RegisterComponent();

  // Cancel Button (ZOrder: 1)
  m_BtnCancel = NewObject<UIBoxButtonComponent>(this);
  m_BtnCancel->SetSize(200.0f, 45.0f);
  m_BtnCancel->SetColors(FColor{40, 45, 55}, FColor{0, 120, 215}, FColor{0, 90, 160});
  m_BtnCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnCancel->SetPivot({0.5f, 0.5f});
  m_BtnCancel->SetAnchoredPosition({120.0f, 80.0f});
  m_BtnCancel->RegisterComponent();

  m_TxtCancel = NewObject<UITextComponent>(this);
  m_TxtCancel->SetText("登録しない (ESC)");
  m_TxtCancel->SetColor(FColor{255, 255, 255});
  m_TxtCancel->SetFontSize(16);
  m_TxtCancel->AttachToComponent(m_BtnCancel);
  m_TxtCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtCancel->SetPivot({0.5f, 0.5f});
  m_TxtCancel->SetAnchoredPosition({0.0f, 0.0f});
  m_TxtCancel->RegisterComponent();
}

WNameInputDialog::~WNameInputDialog() {
  if (m_InputHandle != -1) {
    DeleteKeyInput(m_InputHandle);
  }
}

void WNameInputDialog::BeginPlay() {
  AWidgetBase::BeginPlay();

  m_InputHandle = MakeKeyInput(16, TRUE, TRUE, FALSE);
  SetActiveKeyInput(m_InputHandle);

  m_BtnSubmit->Navigation.Right = m_BtnCancel;
  m_BtnCancel->Navigation.Left = m_BtnSubmit;

  SetFocusedButton(m_BtnSubmit);

  m_BtnSubmit->SetOnPressed([this]() {
    if (m_InputHandle != -1) {
      char buf[256] = {0};
      GetKeyInputString(buf, m_InputHandle);
      std::string name(buf);
      std::string cleanName = KeepAlphanumericOnly(name);
      if (!cleanName.empty() && m_Callback) {
        m_Callback(cleanName);
      }
    }
  });

  m_BtnCancel->SetOnPressed([this]() {
    if (m_CancelCallback) {
      m_CancelCallback();
    }
  });
}

void WNameInputDialog::OnUpdate(float DeltaTime) {
  AWidgetBase::OnUpdate(DeltaTime);

  if (m_InputHandle != -1) {
    char buf[256] = {0};
    GetKeyInputString(buf, m_InputHandle);
    std::string name(buf);

    std::string cleanName = KeepAlphanumericOnly(name);
    if (cleanName != name) {
      SetKeyInputString(const_cast<char*>(cleanName.c_str()), m_InputHandle);
      name = cleanName;
    }

    std::string displayText = name;

    displayText += "|";
    m_InputText->SetText(displayText);
  }

  if (m_InputHandle != -1) {
    int state = CheckKeyInput(m_InputHandle);
    if (state == 1) {
      char buf[256] = {0};
      GetKeyInputString(buf, m_InputHandle);
      std::string name(buf);
      std::string cleanName = KeepAlphanumericOnly(name);
      if (!cleanName.empty() && m_Callback) {
        m_Callback(cleanName);
      } else {
        SetActiveKeyInput(m_InputHandle);
      }
    } else if (state == 2) {
      if (m_CancelCallback) {
        m_CancelCallback();
      }
    }
  }
}

void WNameInputDialog::SetOnNameConfirmed(FOnNameConfirmed callback) { m_Callback = callback; }

void WNameInputDialog::SetOnCancelled(FOnNameInputCancelled callback) {
  m_CancelCallback = callback;
}
