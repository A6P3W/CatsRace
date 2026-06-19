#include "WNameSelectDialog.h"
#include <UITextComponent.h>
#include <UIBoxButton.h>
#include <SpriteComponent.h>
#include <DxLib.h>

WNameSelectDialog::WNameSelectDialog()
{
	// Background Panel (ZOrder: 0)
	auto bgPanel = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
	m_BgPanel = bgPanel.get();
	m_BgPanel->SetRelativeLocation({ 960.0f - 300.0f, 540.0f - 175.0f });
	m_BgPanel->SubmitBox(600, 350, GetColor(15, 15, 20), true, 220);
	m_BgPanel->SetParentComponent(nullptr);
	AddComponent(std::move(bgPanel));

	// Title Text (ZOrder: 1)
	auto titleText = std::make_unique<UITextComponent>("ランキングへの登録方法", 0xFFFFFF, 28);
	m_TitleText = titleText.get();
	m_TitleText->SetAnchor(EUIAnchor::MiddleCenter);
	m_TitleText->SetPivot({ 0.5f, 0.5f });
	m_TitleText->SetAnchoredPosition({ 0.0f, -100.0f });
	m_TitleText->SetParentComponent(nullptr);
	AddComponent(std::move(titleText));

	// 1. Use Existing Name Button
	auto btnUseExisting = std::make_unique<UIBoxButtonComponent>(400.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnUseExisting = btnUseExisting.get();
	m_BtnUseExisting->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnUseExisting->SetPivot({ 0.5f, 0.5f });
	m_BtnUseExisting->SetAnchoredPosition({ 0.0f, -20.0f });
	m_BtnUseExisting->SetParentComponent(nullptr);

	auto txtUseExisting = std::make_unique<UITextComponent>("前回の名前を使用", 0xFFFFFF, 20);
	m_TxtUseExisting = txtUseExisting.get();
	m_TxtUseExisting->SetParentComponent(m_BtnUseExisting);
	m_TxtUseExisting->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtUseExisting->SetPivot({ 0.5f, 0.5f });
	m_TxtUseExisting->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtUseExisting));
	AddComponent(std::move(btnUseExisting));

	// 2. Enter New Name Button
	auto btnEnterNew = std::make_unique<UIBoxButtonComponent>(400.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnEnterNew = btnEnterNew.get();
	m_BtnEnterNew->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnEnterNew->SetPivot({ 0.5f, 0.5f });
	m_BtnEnterNew->SetAnchoredPosition({ 0.0f, 40.0f });
	m_BtnEnterNew->SetParentComponent(nullptr);

	auto txtEnterNew = std::make_unique<UITextComponent>("新しい名前で登録", 0xFFFFFF, 20);
	m_TxtEnterNew = txtEnterNew.get();
	m_TxtEnterNew->SetParentComponent(m_BtnEnterNew);
	m_TxtEnterNew->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtEnterNew->SetPivot({ 0.5f, 0.5f });
	m_TxtEnterNew->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtEnterNew));
	AddComponent(std::move(btnEnterNew));

	// 3. Skip Button
	auto btnSkip = std::make_unique<UIBoxButtonComponent>(400.0f, 45.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnSkip = btnSkip.get();
	m_BtnSkip->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnSkip->SetPivot({ 0.5f, 0.5f });
	m_BtnSkip->SetAnchoredPosition({ 0.0f, 100.0f });
	m_BtnSkip->SetParentComponent(nullptr);

	auto txtSkip = std::make_unique<UITextComponent>("登録しない", 0xAAAAAA, 20);
	m_TxtSkip = txtSkip.get();
	m_TxtSkip->SetParentComponent(m_BtnSkip);
	m_TxtSkip->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtSkip->SetPivot({ 0.5f, 0.5f });
	m_TxtSkip->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(txtSkip));
	AddComponent(std::move(btnSkip));
}

void WNameSelectDialog::BeginPlay()
{
	AWidgetBase::BeginPlay();

	// Navigation link setup (vertical loop)
	m_BtnUseExisting->Navigation.Down = m_BtnEnterNew;
	m_BtnUseExisting->Navigation.Up = m_BtnSkip;

	m_BtnEnterNew->Navigation.Down = m_BtnSkip;
	m_BtnEnterNew->Navigation.Up = m_BtnUseExisting;

	m_BtnSkip->Navigation.Down = m_BtnUseExisting;
	m_BtnSkip->Navigation.Up = m_BtnEnterNew;

	// Initial focus setup
	SetFocusedButton(m_BtnUseExisting);

	// Button callback setup
	m_BtnUseExisting->OnPressed = [this]() {
		if (m_Callback) {
			m_Callback(ENameSelectResult::UseExisting);
		}
	};

	m_BtnEnterNew->OnPressed = [this]() {
		if (m_Callback) {
			m_Callback(ENameSelectResult::EnterNew);
		}
	};

	m_BtnSkip->OnPressed = [this]() {
		if (m_Callback) {
			m_Callback(ENameSelectResult::Skip);
		}
	};
}

void WNameSelectDialog::SetExistingName(const std::string& name)
{
	m_ExistingName = name;
	if (m_TxtUseExisting) {
		m_TxtUseExisting->SetText("登録名：" + name);
	}
}

void WNameSelectDialog::SetOnResult(FOnNameSelectResult callback)
{
	m_Callback = callback;
}
