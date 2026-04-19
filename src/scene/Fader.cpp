#include <DxLib.h>
#include "Fader.h"
#include "core/Application.h"

Fader::Fader()
{
	stat = E_FADE_STAT_ID::E_STAT_FADE_NON;
	alpha = 0.0f;
	endFlg = true;
}

Fader::~Fader()
{
}

bool Fader::SystemInit()
{
	GameInit();
	return true;
}

void Fader::GameInit()
{
	stat = E_FADE_STAT_ID::E_STAT_FADE_NON;
	alpha = 0.0f;
	endFlg = true;
}

void Fader::Update()
{
	if (endFlg) return;

	switch (stat)
	{
	case E_STAT_FADE_NON:
		break;
	case E_STAT_FADE_OUT:
		alpha += FADE_SPEED_ALPHA;
		if (alpha > 255.0f)
		{
			alpha = 255.0f;
			endFlg = true;
			stat = E_FADE_STAT_ID::E_STAT_FADE_NON;
		}
		break;
	case E_STAT_FADE_IN:
		alpha -= FADE_SPEED_ALPHA;
		if (alpha < 0.0f)
		{
			alpha = 0.0f;
			endFlg = true;
			stat = E_FADE_STAT_ID::E_STAT_FADE_NON;
		}
		break;
	default:
		break;
	}
}

void Fader::Draw()
{
	switch (stat)
	{
	case E_STAT_FADE_NON:
		break;
	case E_STAT_FADE_OUT:
	case E_STAT_FADE_IN:
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)alpha);
		DrawBox(0, 0, Application::SCREEN_WID, Application::SCREEN_HIG, GetColor(0, 0, 0), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;
	default:
		break;
	}
}

bool Fader::Release()
{
	return true;
}

E_FADE_STAT_ID Fader::GetNowState()
{
	return stat;
}

bool Fader::IsEnd()
{
	return endFlg;
}

void Fader::SetFade(E_FADE_STAT_ID id)
{
	stat = id;
	if (stat != E_STAT_FADE_NON)
	{
		endFlg = false;
	}
}