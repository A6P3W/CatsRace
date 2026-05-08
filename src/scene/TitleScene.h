#pragma once
#include <memory>
#include "scene/BaseScene.h"
#include "inubasiri/player.h"
#include "inubasiri/map.h"

#include "core/StDefine.h"

class TitleScene : public BaseScene
{
public:
	static constexpr int TITLE_SIZE_X = 600;
	static constexpr int TITLE_SIZE_Y = 250;
	TitleScene();
  ~TitleScene() override;
	bool SystemInit() override; // システム初期化
	bool SceneInit() override; // シーン初期化
	void Update() override;     // 更新
	void Draw() override;       // 描画
	bool Release() override;    // システム解放

private:
	std::unique_ptr<Player> player;
	Map map; 

	int TitleImage; // タイトル画像
	int image; // タイトル画像のハンドル
	int bgImage; // 背景画像（images/world.jpg）のハンドル [追加]

	int Y = 0;
};