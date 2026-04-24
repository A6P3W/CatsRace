#include <DxLib.h>
#include "SceneManager.h"
#include "scene/BaseScene.h"
#include "scene/TitleScene.h"
#include "scene/SceneA.h"
#include "scene/Fader.h"
#include "manager/DebugManager.h"
#include "manager/InputMapper.h"
#include "manager/InputManager.h"

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
        if (currentScene)
		{
			currentScene->Update();
		}

		if (scene_ID == E_SCENE_TITLE && InputManager::GetInstance().GetKeyPressStart(KEY_INPUT_1))
		{
			sceneChangeFlg = true;
			waitScene = E_SCENE_A;
			fader->SetFade(E_STAT_FADE_OUT);
		}
		if (scene_ID != E_SCENE_TITLE && InputManager::GetInstance().GetKeyPressStart(KEY_INPUT_0))
		{
			sceneChangeFlg = true;
			waitScene = E_SCENE_TITLE;
			fader->SetFade(E_STAT_FADE_OUT);
		}
	}
	return true;
}

void SceneManager::Draw()
{
    if (currentScene)
	{
		currentScene->Draw();
	}
	fader->Draw();
}

bool SceneManager::Release()
{
    if (scene_ID != E_SCENE_NON)
	{
		if (!ReleaseScene(scene_ID)) return false;
	}
   if (fader)
	{
		if (!fader->Release()) return false;
       fader.reset();
	}
	return true;
}

bool SceneManager::ReleaseScene(E_SCENE_ID id)
{
 if (id != scene_ID)
	{
		return true;
	}

	if (currentScene)
	{
		if (!currentScene->Release()) return false;
		currentScene.reset();
	}
	return true;
}

bool SceneManager::ChangeScene(E_SCENE_ID sceneID)
{
 DSetLog(5.0f, "scene_id={}", static_cast<int>(scene_ID));
	switch (sceneID) {
	case E_SCENE_TITLE:
      currentScene = std::make_unique<TitleScene>();
		SetTransColor(0xff, 0x00, 0xff);
		if (!currentScene->SystemInit()) return false;
		if (!currentScene->SceneInit()) return false;
		
		break;
	case E_SCENE_A:
        currentScene = std::make_unique<SceneA>();
		if (!currentScene->SystemInit()) return false;
		if (!currentScene->SceneInit()) return false;
		break;
	default:
		return false;

	}
	return true;
}