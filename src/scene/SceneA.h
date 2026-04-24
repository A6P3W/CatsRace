#pragma once
#include <memory>
#include "scene/BaseScene.h"

class SceneA : public BaseScene
{
public:
	static constexpr int TITLE_SIZE_X = 600;
	static constexpr int TITLE_SIZE_Y = 250;
	SceneA();
	~SceneA() override;
	bool SystemInit() override; // システム初期化
	bool SceneInit() override; // シーン初期化
	void Update() override;     // 更新
	void Draw() override;       // 描画
	bool Release() override;    // システム解放

private:
};