#pragma once
#include <memory>
class SceneManager; // 前方宣言

class Application {
public:
	static constexpr int SCREEN_WID = 800; // ゲーム画面の横サイズ
	static constexpr int SCREEN_HIG = 600; // ゲーム画面の縦サイズ

	Application();
	~Application();
	
	bool SystemInit(); // システム初期化
	bool Run(); // メインループ
	bool Release(); // システム解放

private:
	std::unique_ptr<SceneManager> sceneMng; // シーンマネージャー
	bool Update(); // 更新
	void Draw(); // 描画
};