#pragma once

#include "core/StDefine.h"

class Fader
{
public:
	static constexpr float FADE_SPEED_ALPHA = 4; // フェード速度の基準値

	Fader();
	~Fader();
	bool SystemInit();
	void GameInit();
	void Update();
	void Draw();
	bool Release();

	E_FADE_STAT_ID GetNowState();
	bool IsEnd();
	void SetFade(E_FADE_STAT_ID id);

private:
	E_FADE_STAT_ID stat;
	float alpha;
	bool endFlg;
};