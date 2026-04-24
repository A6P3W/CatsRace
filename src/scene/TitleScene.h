#pragma once
#include "inubasiri/player.h"

#include "core/StDefine.h"

class TitleScene
{
public:
	static constexpr int TITLE_SIZE_X = 600;
	static constexpr int TITLE_SIZE_Y = 250;
	TitleScene();
	~TitleScene();
	bool SystemInit(); // システム初期化
	bool SceneInit(); // シーン初期化
	void Update();     // 更新
	void Draw();       // 描画
	bool Release();    // システム解放
	E_SCENE_ID GetNextScene();// 次のシーンID取得

private:
	std::unique_ptr<Player> player;
	int TitleImage; // タイトル画像
	E_SCENE_ID nextScene; // 次のシーンID
	int image; // タイトル画像のハンドル
	int bgImage; // 背景画像（images/world.jpg）のハンドル [追加]

	int Y = 0;
};