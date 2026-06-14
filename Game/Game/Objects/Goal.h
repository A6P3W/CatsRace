#pragma once
#include "Actor.h"

class AGoalActor : public AActor
{
public:
	DEFINE_ACTOR_CLASS(AGoalActor);
    AGoalActor(FVector2D location, FRotator rotation);

    // ���̃A�N�^�[�i�v���C���[�j���d�Ȃ������ɌĂ΂��
    void BeginOverlap(AActor* OtherActor) override;
};