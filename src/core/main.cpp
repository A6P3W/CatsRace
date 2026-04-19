#include <DxLib.h>
#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application app;
	
	// ‰Šú‰»ˆ—
	if (!app.SystemInit()) return -1;

	// ƒQ[ƒ€‹N“®
	app.Run();

	// ‰ğ•úˆ—
	app.Release();

	return 0;
}