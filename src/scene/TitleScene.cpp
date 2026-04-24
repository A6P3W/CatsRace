#include <Dxlib.h>
#include "manager/ResourceManager.h"
#include "scene/TitleScene.h"
#include "core/Application.h"
#include "inubasiri/player.h"

TitleScene::TitleScene()
{
	TitleImage = -1;
	image = -1;
	bgImage = -1; // 初期化 [追加]
}

TitleScene::~TitleScene()
{
}

bool TitleScene::SystemInit()
{
	return true;
}

bool TitleScene::SceneInit()
{
	// 既存の画像読み込み
	image = ResourceManager::GetInstance().GetGraph("images/arroaw-up.png");
	// 背景画像の読み込み
	bgImage = ResourceManager::GetInstance().GetGraph("images/road.png");
	//プレイヤー生成
	player = std::make_unique<Player>();
	return true;
}

void TitleScene::Update()
{
	if (player) {
		player->Update(); // 複素数による移動・回転が実行される
	}
}

void TitleScene::Draw()
{
	float screenW = Application::SCREEN_WID;
	float screenH = Application::SCREEN_HIG;

	// 描画モードをバイリニアに設定
	SetDrawMode(DX_DRAWMODE_BILINEAR);

	// 修正案：極端な変形を避け、パース補正を期待できる描画方法を検討する
	// 現状の DrawModiGraph では「波打ち」は原理上避けられません

	DrawModiGraph(
		0, Y+screenH * 0.25f,
		screenW, Y+screenH * 0.25f,
		screenW + screenW, Y+screenH,
		0 - screenW, Y+screenH,
		bgImage, TRUE
	);

	DrawGraph(10, 10, image, TRUE);

	if (player) {
		player->Draw(); // 座標のデバッグ表示など
	}
}

bool TitleScene::Release()
{
	// ResourceManagerが解放を管理するため、ここでは特になし
	return true;
}
