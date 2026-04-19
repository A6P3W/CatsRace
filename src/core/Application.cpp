#include <Dxlib.h>
#include <time.h>
#include "Application.h"
#include "manager/SceneManager.h"
#include "manager/InputManager.h"
#include "manager/InputMapper.h"
#include "manager/ResourceManager.h"
#include <memory>
#include <iostream>
Application::Application() 
{
	sceneMng = nullptr;
}

Application::~Application() 
{
}

bool Application::SystemInit() 
{
	SetWindowText("AGS_2026_Summer"); // ウィンドウタイトル設定
	SetGraphMode(SCREEN_WID, SCREEN_HIG, 32); // 画面サイズ設定
	ChangeWindowMode(true); // ウィンドウモードに設定
	
	// DXライブラリ初期化
	if (DxLib_Init() == -1) return false;
	
	// 乱数初期化
	SRand((unsigned int)time(NULL));
	if (!ResourceManager::GetInstance().Init()) return false; // リソースマネージャーの初期化
	// シーンマネージャーの生成と初期化
	sceneMng = std::make_unique<SceneManager>();
	if (sceneMng == nullptr) return false;
	if (!sceneMng->SystemInit()) return false;
	
	

	return true;
}

bool Application::Run() 
{
	// メインループ
	while (ProcessMessage() == 0 && !InputMapper::GetInstance().GetActionStart(E_INPUT_ACTION::CANCEL))
	{
		// 更新
		if (Update() == false) return false;
		// 描画
		Draw();
	}
	
	return true;
}

bool Application::Update() 
{
	InputManager::GetInstance().Update();
	if (!sceneMng->Update()) return false;

	return true;
}

void Application::Draw() 
{
	SetDrawScreen(DX_SCREEN_BACK); // 裏画面を描画対象に設定
	ClearDrawScreen(); // 画面クリア
	sceneMng->Draw();
	ScreenFlip(); // 裏画面と表画面の入れ替え
	
}

bool Application::Release() 
{
	// シーンマネージャーの解放
	if (sceneMng != nullptr) {
		if (!sceneMng->Release()) return false;
	}
	return (DxLib_End() == 0); // DXライブラリの後始末
}