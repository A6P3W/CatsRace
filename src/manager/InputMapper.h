#pragma once
#include <map>
#include <vector>
#include "core/StDefine.h"

class InputMapper {
public:
    static InputMapper& GetInstance() {
        static InputMapper instance;
        return instance;
    }


    // アクション名での判定
    bool GetKeyPressStart(E_INPUT_ACTION action);
    bool GetKeyPressing(E_INPUT_ACTION action);
	bool GetKeyRelease(E_INPUT_ACTION action);

private:
	InputMapper();
    // アクションに対して複数のキーを割り当てられるように vector を使用
    std::map<E_INPUT_ACTION, std::vector<int>> keyBindings;
};