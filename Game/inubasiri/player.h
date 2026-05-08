#pragma once
#include <complex>
#include "object/GameObject.h"

using Vector2D = std::complex<float>;

class Player :public GameObject {
public:
	Player();
	void Update()override;
	void Draw() override;

	Vector2D GetPosition() const { return pos; }
	Vector2D GetDirection() const { return dir; }

private:
	Vector2D pos;
	Vector2D dir;
	float speed;
	int graphHandle;

	const float MAX_SPEED = 10.0f;
	const float TURN_ANGLE = 0.01f;

};