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
  auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
  m_BgPanel = bgPanel.get();
  m_BgPanel->SetRelativeLocation({960.0f - 300.0f, 540.0f - 175.0f});
  m_BgPanel->SubmitBox(600, 350, GetColor(15, 15, 20), true, 220);
  m_BgPanel->SetParentComponent(nullptr);
  AddComponent(std::move(bgPanel));

  // Title Text (ZOrder: 1)
  auto titleText = std::make_unique<UITextComponent>("登録名を入力してください", 0xFFFFFF, 24);
  m_TitleText = titleText.get();
  m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
  m_TitleText->SetPivot({0.5f, 0.5f});
  m_TitleText->SetAnchoredPosition({0.0f, -100.0f});
  m_TitleText->SetParentComponent(nullptr);
  AddComponent(std::move(titleText));

  // Input Box Background (ZOrder: 1)
  auto inputBoxBg = std::make_unique<MSpriteComponent>(1, RenderSpace::Screen);
  m_InputBoxBg = inputBoxBg.get();
  m_InputBoxBg->SetRelativeLocation({960.0f - 200.0f, 540.0f - 35.0f});
  m_InputBoxBg->SubmitBox(400, 50, GetColor(30, 35, 45), true, 255);
  m_InputBoxBg->SetParentComponent(nullptr);
  AddComponent(std::move(inputBoxBg));

  // Input Box Text Component (ZOrder: 2)
  auto inputText = std::make_unique<UITextComponent>("", 0xFFFFFF, 24);
  m_InputText = inputText.get();
  m_InputText->SetParentComponent(nullptr);
  m_InputText->SetPivot({0.5f, 0});
  m_InputText->SetAnchoredPosition({0.0f, 0.0f});  // Left margin inside box
  AddComponent(std::move(inputText));

  // Submit Button (ZOrder: 1)
  auto btnSubmit = std::make_unique<UIBoxButtonComponent>(
      200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnSubmit = btnSubmit.get();
  m_BtnSubmit->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnSubmit->SetPivot({0.5f, 0.5f});
  m_BtnSubmit->SetAnchoredPosition({-120.0f, 80.0f});

  auto txtSubmit = std::make_unique<UITextComponent>("送信 (ENTER)", 0xFFFFFF, 18);
  m_TxtSubmit = txtSubmit.get();
  m_TxtSubmit->SetParentComponent(m_BtnSubmit);
  m_TxtSubmit->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtSubmit->SetPivot({0.5f, 0.5f});
  m_TxtSubmit->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(txtSubmit));
  AddComponent(std::move(btnSubmit));

  // Cancel Button (ZOrder: 1)
  auto btnCancel = std::make_unique<UIBoxButtonComponent>(
      200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160)
  );
  m_BtnCancel = btnCancel.get();
  m_BtnCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_BtnCancel->SetPivot({0.5f, 0.5f});
  m_BtnCancel->SetAnchoredPosition({120.0f, 80.0f});

  auto txtCancel = std::make_unique<UITextComponent>("登録しない (ESC)", 0xFFFFFF, 16);
  m_TxtCancel = txtCancel.get();
  m_TxtCancel->SetParentComponent(m_BtnCancel);
  m_TxtCancel->SetAnchor(EUIAnchor::MiddleCenter);
  m_TxtCancel->SetPivot({0.5f, 0.5f});
  m_TxtCancel->SetAnchoredPosition({0.0f, 0.0f});

  AddComponent(std::move(txtCancel));
  AddComponent(std::move(btnCancel));
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

  m_BtnSubmit->OnPressed = [this]() {
    if (m_InputHandle != -1) {
      char buf[256] = {0};
      GetKeyInputString(buf, m_InputHandle);
      std::string name(buf);
      std::string cleanName = KeepAlphanumericOnly(name);
      if (!cleanName.empty() && m_Callback) {
        m_Callback(cleanName);
      }
    }
  };

  m_BtnCancel->OnPressed = [this]() {
    if (m_CancelCallback) {
      m_CancelCallback();
    }
  };
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
