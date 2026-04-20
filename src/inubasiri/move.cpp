#include <DxLib.h>
#include "move.h"
#include "manager/InputManager.h"
#include <iostream>
Mover::Mover(float x, float y, float vx, float vy) : pos(x, y), velocity(vx, vy) {}

void Mover::Update() {
	if (InputManager::GetInstance().GetKeyPressing(KEY_INPUT_A)){
		RotateVelocity(-0.1f);
		std::cout << "a" << std::endl;
	}
	if (InputManager::GetInstance().GetKeyPressing(KEY_INPUT_D)) {
		RotateVelocity(0.1f); 
	}
	pos += velocity;
}

void Mover::RotateVelocity(float angle) {
	Vector2D rotation(std::polar(1.0f, angle));
	velocity *= rotation;
}