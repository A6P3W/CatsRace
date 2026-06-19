#include "WPauseMenu.h"
#include <UITextComponent.h>
#include <UIBoxButton.h>
#include <SpriteComponent.h>
#include <DxLib.h>


WPauseMenu::WPauseMenu()
{
	// 1. 一時停止中でも更新を許可する
	SetUpdateableAnytime(true);

	// 2. 画面全体を覆う半透明背景
	auto bgWidget = std::make_unique<MSpriteComponent>();
	bgWidget->SubmitBox(1920.0f, 1080.0f, GetColor(20, 20, 20), true, 220);
	auto* bgWidgetPtr = bgWidget.get();
	AddComponent(std::move(bgWidget));

	auto bgSprite = std::make_unique<MSpriteComponent>(-10, RenderSpace::Screen);
	bgSprite->SetParentComponent(bgWidgetPtr);
	bgSprite->SetRelativeLocation({ -1920.0f * 0.5f, -1080.0f * 0.5f });
	bgSprite->SubmitBox(1920.0f, 1080.0f, GetColor(0, 0, 0), true, 180);
	AddComponent(std::move(bgSprite));

	// 3. タイトルテキスト "PAUSE"
	auto txtTitle = std::make_unique<UITextComponent>("PAUSE", 0xFFFFFF, 64);
	m_TxtTitle = txtTitle.get();
	m_TxtTitle->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtTitle->SetPivot({ 0.5f, 0.5f });
	m_TxtTitle->SetAnchoredPosition({ 0.0f, -250.0f });
	AddComponent(std::move(txtTitle));

	// 4. 「再開」ボタン
	auto btnResume = std::make_unique<UIBoxButtonComponent>(350.0f, 70.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnResume = btnResume.get();
	m_BtnResume->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnResume->SetPivot({ 0.5f, 0.5f });
	m_BtnResume->SetAnchoredPosition({ 0.0f, -50.0f });

	auto txtResume = std::make_unique<UITextComponent>("再開", 0xFFFFFF, 24);
	m_TxtResume = txtResume.get();
	m_TxtResume->SetParentComponent(m_BtnResume);
	m_TxtResume->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtResume->SetPivot({ 0.5f, 0.5f });
	m_TxtResume->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(btnResume));
	AddComponent(std::move(txtResume));

	// 5. 「最初から開始」ボタン
	auto btnRestart = std::make_unique<UIBoxButtonComponent>(350.0f, 70.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnRestart = btnRestart.get();
	m_BtnRestart->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnRestart->SetPivot({ 0.5f, 0.5f });
	m_BtnRestart->SetAnchoredPosition({ 0.0f, 50.0f });

	auto txtRestart = std::make_unique<UITextComponent>("最初から開始", 0xFFFFFF, 24);
	m_TxtRestart = txtRestart.get();
	m_TxtRestart->SetParentComponent(m_BtnRestart);
	m_TxtRestart->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtRestart->SetPivot({ 0.5f, 0.5f });
	m_TxtRestart->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(btnRestart));
	AddComponent(std::move(txtRestart));

	// 6. 「タイトルへ戻る」ボタン
	auto btnTitle = std::make_unique<UIBoxButtonComponent>(350.0f, 70.0f, GetColor(40, 45, 55), GetColor(0, 120, 215), GetColor(0, 90, 160));
	m_BtnTitle = btnTitle.get();
	m_BtnTitle->SetAnchor(EUIAnchor::MiddleCenter);
	m_BtnTitle->SetPivot({ 0.5f, 0.5f });
	m_BtnTitle->SetAnchoredPosition({ 0.0f, 150.0f });

	auto txtTitleBtn = std::make_unique<UITextComponent>("タイトルへ戻る", 0xFFFFFF, 24);
	m_TxtTitleBtn = txtTitleBtn.get();
	m_TxtTitleBtn->SetParentComponent(m_BtnTitle);
	m_TxtTitleBtn->SetAnchor(EUIAnchor::MiddleCenter);
	m_TxtTitleBtn->SetPivot({ 0.5f, 0.5f });
	m_TxtTitleBtn->SetAnchoredPosition({ 0.0f, 0.0f });

	AddComponent(std::move(btnTitle));
	AddComponent(std::move(txtTitleBtn));
}

void WPauseMenu::BeginPlay()
{
	AWidgetBase::BeginPlay();

	// ボタン間のナビゲーション設定 (上下移動)
	m_BtnResume->Navigation.Down = m_BtnRestart;
	
	m_BtnRestart->Navigation.Up = m_BtnResume;
	m_BtnRestart->Navigation.Down = m_BtnTitle;

	m_BtnTitle->Navigation.Up = m_BtnRestart;

	// 初期フォーカス
	SetFocusedButton(m_BtnResume);

	// ボタンのコールバック
	m_BtnResume->OnPressed = [this]() {
		if (OnResumePressed) OnResumePressed();
	};

	m_BtnRestart->OnPressed = [this]() {
		if (OnRestartPressed) OnRestartPressed();
	};

	m_BtnTitle->OnPressed = [this]() {
		if (OnTitlePressed) OnTitlePressed();
	};
}

void WPauseMenu::Cancel()
{
	// キャンセル（ESCキーなど）で再開する
	if (OnResumePressed) OnResumePressed();
}
