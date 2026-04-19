#include "InputMapper.h"
#include "InputManager.h"
#include <DxLib.h>

InputMapper::InputMapper() {
    keyBindings[E_INPUT_ACTION::CANCEL] = { KEY_INPUT_ESCAPE };
	keyBindings[E_INPUT_ACTION::UP] = { KEY_INPUT_UP, KEY_INPUT_W };
}

bool InputMapper::GetActionStart(E_INPUT_ACTION action) {
    for (int keyCode : keyBindings[action]) {
        if (InputManager::GetInstance().GetKeyPressStart(keyCode)) return true;
    }
    return false;
}