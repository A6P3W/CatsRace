#include "WNameInputDialog.h"
#include <UITextComponent.h>
#include <UIBoxButton.h>
#include <SpriteComponent.h>
#include <DxLib.h>

WNameInputDialog::WNameInputDialog()
{
	// Background Panel (ZOrder: 0)
	auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
	m_BgPanel = bgPanel.get();
	m_BgPanel->SetRelativeLocation({ 960.0f - 300.0f, 540.0f - 175.0f });
	m_BgPanel->SubmitBox(600, 350, GetColor(15, 15, 20), true, 220);
	m_BgPanel->SetParentComponent(nullptr);
	AddComponent(std::move(bgPanel));

	// Title Text (ZOrder: 1)
	auto titleText = std::make_unique<UITextComponent>("Enter your name (Max 16 chars)", 0xFFFFFF, 24);
	m_TitleText = titleText.get();
	m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
	m_TitleText->SetPivot({ 0.5f, 0.5f });
	m_TitleText->SetAnchoredPosition({ 0.0f, -100.0f });
	m_TitleText->SetParentComponent(nullptr);
	AddComponent(std::move(titleText));

	// Input Box Background (ZOrder: 1)
	auto inputBoxBg = std::make_unique<MSpriteComponent>(1, RenderSpace::Screen);
	m_InputBoxBg = inputBoxBg.get();
	m_InputBoxBg->SetRelativeLocation({ 960.0f - 200.0f, 540.0f - 35.0f });
	m_InputBoxBg->SubmitBox(400, 50, GetColor(30, 35, 45), true, 255);
	m_InputBoxBg->SetParentComponent(nullptr);
	AddComponent(std::move(inputBoxBg));

	// Input Box Text Component (ZOrder: 2)
	auto inputText = std::make_unique<UITextComponent>("", 0xFFFFFF, 24);
	m_InputText = inputText.get();
	m_InputText->SetParentComponent(nullptr);
	m_InputText->SetPivot({ 0.5f, 0 });
	m_InputText->SetAnchoredPosition({ 0.0f, 0.0f }); // Left margin inside box
	AddComponent(std::move(inputText));

	// Submit Button (ZOrder: 1)
	auto btnSubmit = std::make_unique<UIBoxButtonComponent>(200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnSubmit = btnSubmit.get();
	m_BtnSubmit->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnSubmit->SetPivot({ 0.5f, 0.5f });
	m_BtnSubmit->SetAnchoredPosition({ 0.0f, 80.0f });
	m_BtnSubmit->SetParentComponent(nullptr);

	auto txtSubmit = std::make_unique<UITextComponent>("Submit", 0xFFFFFF, 20);
	m_TxtSubmit = txtSubmit.get();
	m_TxtSubmit->SetParentComponent(m_BtnSubmit);
	m_TxtSubmit->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtSubmit->SetPivot({ 0.5f, 0.5f });
	m_TxtSubmit->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtSubmit));
	AddComponent(std::move(btnSubmit));
}

WNameInputDialog::~WNameInputDialog()
{
	if (m_InputHandle != -1) {
		DeleteKeyInput(m_InputHandle);
	}
}

void WNameInputDialog::BeginPlay()
{
	AWidgetBase::BeginPlay();

	// Create input handle (Max 16 chars, Kanji disabled, Single-byte input forced)
	m_InputHandle = MakeKeyInput(16, TRUE, FALSE, FALSE);
	SetActiveKeyInput(m_InputHandle);

	// Set initial focus
	SetFocusedButton(m_BtnSubmit);

	// Button callback setup
	m_BtnSubmit->OnPressed = [this]() {
		if (m_InputHandle != -1) {
			char buf[256] = {0};
			GetKeyInputString(buf, m_InputHandle);
			std::string name(buf);
			if (!name.empty() && m_Callback) {
				m_Callback(name);
			}
		}
	};
}

void WNameInputDialog::OnUpdate(float DeltaTime)
{
	AWidgetBase::OnUpdate(DeltaTime);

	// Get current typed characters and update display with blink cursor
	if (m_InputHandle != -1) {
		char buf[256] = {0};
		GetKeyInputString(buf, m_InputHandle);
		std::string name(buf);

		m_BlinkTimer += DeltaTime;
		if (m_BlinkTimer >= 0.5f) {
			m_BlinkTimer -= 0.5f;
			m_bShowCursor = !m_bShowCursor;
		}

		std::string displayText = name;
		if (m_bShowCursor) {
			displayText += "|";
		}
		if (m_InputText) {
			m_InputText->SetText(displayText);
		}
	}

	// Check Enter key confirmation
	if (m_InputHandle != -1 && CheckKeyInput(m_InputHandle) == 1) {
		char buf[256] = {0};
		GetKeyInputString(buf, m_InputHandle);
		std::string name(buf);
		if (!name.empty() && m_Callback) {
			m_Callback(name);
		} else {
			SetActiveKeyInput(m_InputHandle);
		}
	}
}

void WNameInputDialog::SetOnNameConfirmed(FOnNameConfirmed callback)
{
	m_Callback = callback;
}
