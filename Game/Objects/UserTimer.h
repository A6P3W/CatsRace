#pragma once
#include <Actor.h>
class UserTimer : public AActor
{
public:
	void OnUpdate(float DeltaTime) override;

private:
	float m_timer = 0.0f;
};

