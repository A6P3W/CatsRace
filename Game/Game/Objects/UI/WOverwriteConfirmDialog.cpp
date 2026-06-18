#include "WOverwriteConfirmDialog.h"
#include <UITextComponent.h>
#include <UIBoxButton.h>
#include <SpriteComponent.h>
#include <DxLib.h>

WOverwriteConfirmDialog::WOverwriteConfirmDialog()
{
	// Background Panel (ZOrder: 0)
	auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
	m_BgPanel = bgPanel.get();
	m_BgPanel->SetRelativeLocation({ 960.0f - 300.0f, 540.0f - 150.0f });
	m_BgPanel->SubmitBox(600, 300, GetColor(15, 15, 20), true, 220);
	m_BgPanel->SetParentComponent(nullptr);
	AddComponent(std::move(bgPanel));

	// Title Text (Warning style, ZOrder: 1)
	auto titleText = std::make_unique<UITextComponent>("Warning: Duplicate Name", 0xFF5555, 26);
	m_TitleText = titleText.get();
	m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
	m_TitleText->SetPivot({ 0.5f, 0.5f });
	m_TitleText->SetAnchoredPosition({ 0.0f, -80.0f });
	m_TitleText->SetParentComponent(nullptr);
	AddComponent(std::move(titleText));

	// Sub Text (ZOrder: 1)
	auto subText = std::make_unique<UITextComponent>("Overwrite score with this name?", 0xFFFFFF, 18);
	m_SubText = subText.get();
	m_SubText->SetAnchor(EUIAnchor::MiddleCenter);
	m_SubText->SetPivot({ 0.5f, 0.5f });
	m_SubText->SetAnchoredPosition({ 0.0f, -30.0f });
	m_SubText->SetParentComponent(nullptr);
	AddComponent(std::move(subText));

	// 1. Overwrite Button (Left, ZOrder: 1)
	auto btnOverwrite = std::make_unique<UIBoxButtonComponent>(200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnOverwrite = btnOverwrite.get();
	m_BtnOverwrite->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnOverwrite->SetPivot({ 0.5f, 0.5f });
	m_BtnOverwrite->SetAnchoredPosition({ -120.0f, 50.0f });
	m_BtnOverwrite->SetParentComponent(nullptr);

	auto txtOverwrite = std::make_unique<UITextComponent>("Overwrite", 0xFFFFFF, 18);
	m_TxtOverwrite = txtOverwrite.get();
	m_TxtOverwrite->SetParentComponent(m_BtnOverwrite);
	m_TxtOverwrite->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtOverwrite->SetPivot({ 0.5f, 0.5f });
	m_TxtOverwrite->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtOverwrite));
	AddComponent(std::move(btnOverwrite));

	// 2. Re-enter Button (Right, ZOrder: 1)
	auto btnReEnter = std::make_unique<UIBoxButtonComponent>(200.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnReEnter = btnReEnter.get();
	m_BtnReEnter->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnReEnter->SetPivot({ 0.5f, 0.5f });
	m_BtnReEnter->SetAnchoredPosition({ 120.0f, 50.0f });
	m_BtnReEnter->SetParentComponent(nullptr);

	auto txtReEnter = std::make_unique<UITextComponent>("Re-enter", 0xFFFFFF, 18);
	m_TxtReEnter = txtReEnter.get();
	m_TxtReEnter->SetParentComponent(m_BtnReEnter);
	m_TxtReEnter->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtReEnter->SetPivot({ 0.5f, 0.5f });
	m_TxtReEnter->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtReEnter));
	AddComponent(std::move(btnReEnter));
}

void WOverwriteConfirmDialog::BeginPlay()
{
	AWidgetBase::BeginPlay();

	// Navigation setup (horizontal)
	m_BtnOverwrite->Navigation.Right = m_BtnReEnter;
	m_BtnReEnter->Navigation.Left = m_BtnOverwrite;

	// Initial focus setup to prevent accidental overwrites
	SetFocusedButton(m_BtnReEnter);

	// Button callback setup
	m_BtnOverwrite->OnPressed = [this]() {
		if (m_Callback) {
			m_Callback(EOverwriteResult::Overwrite);
		}
	};

	m_BtnReEnter->OnPressed = [this]() {
		if (m_Callback) {
			m_Callback(EOverwriteResult::ReEnter);
		}
	};
}

void WOverwriteConfirmDialog::SetOnResult(FOnOverwriteResult callback)
{
	m_Callback = callback;
}
