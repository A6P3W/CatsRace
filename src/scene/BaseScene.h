#pragma once

#include "core/StDefine.h"

class BaseScene
{
public:
    virtual ~BaseScene() = default;

    virtual bool SystemInit() = 0;
    virtual bool SceneInit() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual bool Release() = 0;
};
