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
	if (!map.Load("images/otamesi.csv", "images/map.bmp")) {
		printfDx("Map Load Failed! Check file path.\n");
		return false;
	}
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
	map.Draw();

	// 描画モードをバイリニアに設定
	SetDrawMode(DX_DRAWMODE_BILINEAR);

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
