#pragma once


class InputManager
{
private:
	InputManager();
	~InputManager();
public:
	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	bool GetKeyPressStart(int keyCode);
	bool GetKeyPressing(int keyCode);
	bool GetKeyRelease(int keyCode);
	void Update();

	int key[256];
	int prevKey[256];
};