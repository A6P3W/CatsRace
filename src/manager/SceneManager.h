#pragma once
#include <memory>
#include "core/StDefine.h"

class Fader;
class BaseScene;

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	bool SystemInit(); // システム初期化
	bool GameInit();   // ゲーム初期化
	bool Update();     // 更新
	void Draw();       // 描画
	bool Release();    // システム解放
	bool ChangeScene(E_SCENE_ID sceneID);
	bool ReleaseScene(E_SCENE_ID id);
private:
    std::unique_ptr<Fader> fader;		// フェーダー
	E_SCENE_ID waitScene; // 待機シーンID
	bool sceneChangeFlg; // シーン変更フラグ
	std::unique_ptr<BaseScene> currentScene;
	E_SCENE_ID scene_ID;	// 現在のシーンID
};
