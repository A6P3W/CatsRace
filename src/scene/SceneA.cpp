#include "SceneA.h"
#include <DxLib.h>

SceneA::SceneA()
{
}

SceneA::~SceneA()
{
}

bool SceneA::SystemInit()
{
    return true;
}

bool SceneA::SceneInit()
{
    return true;
}

void SceneA::Update()
{
}

void SceneA::Draw()
{
    DrawString(20, 20, "SceneA", GetColor(255, 255, 255));
}

bool SceneA::Release()
{
    return true;
}
