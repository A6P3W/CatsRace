#include <DxLib.h>
#include "InputManager.h"

InputManager::InputManager()
{
	for (int i = 0; i < 256; i++)
	{
		key[i] = 0;
       prevKey[i] = 0;
	}
}

InputManager::~InputManager()
{
}

void InputManager::Update()
{
	char tmpKey[256];
	GetHitKeyStateAll(tmpKey);
	for (int i = 0; i < 256; i++)
	{
     prevKey[i] = key[i];

		if (tmpKey[i] != 0)
		{
			key[i]++;
		}
		else
		{
			key[i] = 0;
		}
	}
}

bool InputManager::GetKeyPressStart(int keyCode)
{
	if (key[keyCode] == 1)
	{
		return true;
	}
	return false;
}

bool InputManager::GetKeyPressing(int keyCode)
{
   if (key[keyCode] > 0)
	{
		return true;
	}

	return false;
}

bool InputManager::GetKeyRelease(int keyCode)
{
   if (prevKey[keyCode] > 0 && key[keyCode] == 0)
	{
		return true;
	}

	return false;
}
