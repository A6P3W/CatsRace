#include <DxLib.h>
#include "SceneManager.h"
#include "scene/TitleScene.h"
#include "scene/Fader.h"

SceneManager::SceneManager()
{
	sceneChangeFlg = false;
	scene_ID = waitScene = E_SCENE_ID::E_SCENE_NON;
}

SceneManager::~SceneManager()
{
}

bool SceneManager::SystemInit()
{
    fader = std::make_unique<Fader>();
	if (!fader->SystemInit()) {
		return false;
	}
	sceneChangeFlg = false;
	// 最初のシーンをタイトルシーンに設定
	scene_ID = E_SCENE_ID::E_SCENE_TITLE;
	ChangeScene(scene_ID);
	return true;
}

bool SceneManager::GameInit()
{
	// 使わない可能性あり
	return true;
}

bool SceneManager::Update()
{
	fader->Update();
	if (sceneChangeFlg)
	{
		// フェードアウトが終わったらフェードイン開始
		if (fader->IsEnd() && waitScene != E_SCENE_NON)
		{
			if (!ReleaseScene(scene_ID)) return false;
			scene_ID = waitScene;
			if (!ChangeScene(scene_ID)) return false;
			waitScene = E_SCENE_NON;
			fader->SetFade(E_STAT_FADE_IN);
		}
		else if (fader->IsEnd() && waitScene == E_SCENE_NON)
		{
			sceneChangeFlg = false;
		}
	}
	else
	{
		// 現在のシーンに応じて更新処理を実行
		switch (scene_ID) {
		case E_SCENE_TITLE:

			titleInst->Update();
			// 次のシーンに遷移するときのみ次のシーンIDを取得
			if (titleInst->GetNextScene() != E_SCENE_NON)
			{
				sceneChangeFlg = true;
				waitScene = titleInst->GetNextScene();
				fader->SetFade(E_STAT_FADE_OUT);
				//scene_ID = titleInst->GetNextScene();
			}
			break;
			// 他のシーンの更新処理をここに追加
		default:
			break;
		}
	}
	return true;
}

void SceneManager::Draw()
{
	// 現在のシーンに応じて描画処理を実行
	switch (scene_ID) {
	case E_SCENE_TITLE:
		titleInst->Draw();
		break;
	// 他のシーンの描画処理をここに追加
	default:
		break;
	}
	fader->Draw();
}

bool SceneManager::Release()
{
	ReleaseScene(E_SCENE_TITLE);
   if (fader)
	{
		if (!fader->Release()) return false;
       fader.reset();
	}
	return true;
}

bool SceneManager::ReleaseScene(E_SCENE_ID id)
{
	switch (id)
	{
	case E_SCENE_TITLE:
       if (titleInst)
		{
			if (!titleInst->Release()) return false;
           titleInst.reset();
		}
		break;
	}
	return true;
}

bool SceneManager::ChangeScene(E_SCENE_ID sceneID)
{
	//if (!ReleaseScene(sceneID)) return false;

	switch (sceneID) {
	case E_SCENE_TITLE:
       if (!titleInst)
		{
           titleInst = std::make_unique<TitleScene>();
			SetTransColor(0xff, 0x00, 0xff);
			if (!titleInst->SystemInit()) return false;
			if (!titleInst->SceneInit()) return false;
		}
		break;

	}
	return true;
}