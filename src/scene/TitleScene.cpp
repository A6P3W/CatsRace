#include <Dxlib.h>
#include "manager/ResourceManager.h"
#include "scene/TitleScene.h"
#include "core/Application.h"

TitleScene::TitleScene()
{
	TitleImage = -1;
	image = -1;
	nextScene = E_SCENE_ID::E_SCENE_INIT;
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
	// シーン初期化処理（必要に応じて追加）
	nextScene = E_SCENE_ID::E_SCENE_NON;
	image = ResourceManager::GetInstance().GetGraph("images/arroaw-up.png");

	return true;
}

void TitleScene::Update()
{

}

void TitleScene::Draw()
{
	DrawGraph(10,10,image, TRUE);
}

bool TitleScene::Release()
{
	return true;
}

E_SCENE_ID TitleScene::GetNextScene()
{
	return nextScene;
}

