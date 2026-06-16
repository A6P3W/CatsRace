#pragma once
#include "Actor.h"
#include "UMath.h"
#include <unordered_set>
#include "SoundComponent.h"
class MMovementComponent;

class ASlowFloor : public AActor
{
public:
    DEFINE_ACTOR_CLASS(ASlowFloor);


    ASlowFloor(float width = 300.0f, float height = 300.0f, float slowStrength = 0.85f);

    void OnUpdate(float DeltaTime) override;

    void BeginOverlap(AActor* OtherActor) override;
    void EndOverlap(AActor* OtherActor) override;

    // --- 設定 ---
    void SetSize(float width, float height);
    void SetSlowStrength(float strength) { m_slowStrength = strength; }
    float GetSlowStrength() const { return m_slowStrength; }

private:
    void BeginPlay() override {}

    std::unordered_set<MMovementComponent*> m_overlappingMovements;

    float m_slowStrength = 0.85f; 
    float m_width = 1000.0f;
    float m_height = 300.0f;
    MSoundComponent* m_sound = nullptr;
};