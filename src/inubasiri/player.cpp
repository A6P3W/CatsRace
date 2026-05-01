#include "Player.h"
#include "manager/InputMapper.h"
#include "manager/ResourceManager.h"
#include"manager/DebugManager.h"
#include <DxLib.h>

Player::Player() : pos(400.0f, 300.0f), dir(0.0f, -1.0f), speed(0) {
	graphHandle = ResourceManager::GetInstance().GetGraph("images/arrow-up.png");
}
void Player::Update() {
	auto& input = InputMapper::GetInstance();
	//‰ñ“]
	if (input.GetKeyPressing(E_INPUT_ACTION::LEFT)) {
		// ¶‰ñ“] = ƒxƒNƒgƒ‹‚É std::polar(1.0, -angle) ‚ðŠ|‚¯‚é
		dir *= std::polar(1.0f, -TURN_ANGLE);
	}
	if (input.GetKeyPressing(E_INPUT_ACTION::RIGHT)) {
		// ‰E‰ñ“] = ƒxƒNƒgƒ‹‚É std::polar(1.0, angle) ‚ðŠ|‚¯‚é
		dir *= std::polar(1.0f, TURN_ANGLE);
	}
	// •ûŒüƒxƒNƒgƒ‹‚ð’PˆÊƒxƒNƒgƒ‹‚É•Û‚ÂiŒë·’~Ï–hŽ~j
	dir /= std::abs(dir);
	
	//‰Á‘¬Œ¸‘¬
	if (input.GetKeyPressing(E_INPUT_ACTION::UP)) {
		speed += 0.04f;
	}
	else {
		speed *= 0.97f; //–€ŽC
	}
	if (speed > MAX_SPEED) speed = MAX_SPEED;

	pos += dir * speed;
	DSetLog(speed);
	DSetLog(GetPosition());
	}

void Player::Draw() {
	float angle = std::arg(dir) + 3.141592f / 2.0f;



	DrawRotaGraph(
		(int)pos.real(), (int)pos.imag(), // •`‰æˆÊ’u
		1.0,					          // Šg‘å—¦
		(double)angle,                   // ‰ñ“]Šp“x
		graphHandle,                     // ‰æ‘œƒnƒ“ƒhƒ‹
		TRUE                             // “§‰ßƒtƒ‰ƒO
	);
}
