#pragma once

enum E_SCENE_ID
{
	E_SCENE_INIT = -2,
	E_SCENE_NON = -1,
	E_SCENE_TITLE,			// タイトル画面
	E_SCENE_A,
	E_SCENE_ID_MAX,
};

enum E_FADE_STAT_ID
{
	E_STAT_FADE_NON = -1,
	E_STAT_FADE_OUT,			// フェードアウト
	E_STAT_FADE_IN,				// フェードイン

	E_STAT_FADE_MAX,
};

enum E_INPUT_ACTION
{
	CANCEL,
	UP,
	DOWN, 
	LEFT,  
	RIGHT, 
};