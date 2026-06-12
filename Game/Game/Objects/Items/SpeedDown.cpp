#include "SpeedDown.h"
#include "RectangleCollisionComponent.h"
#include "SpriteComponent.h"
#include "MovementComponent.h"
#include "Log.h"
REGISTER_ACTOR(ASlowFloor);

ASlowFloor::ASlowFloor(float width, float height, float slowStrength)
    : m_width(width)
    , m_height(height)
    , m_slowStrength(slowStrength)
{
    auto col = std::make_unique<MRectangleCollisionComponent>(m_width, m_height);
    col->SetCollisionType(ECollisionType::Overlap); 
    col->SetStatic(true);
    col->SetParentComponent(GetRootComponent());
    AddComponent(std::move(col));

    auto sprite = std::make_unique<MSpriteComponent>(10, RenderSpace::World);
    sprite->SubmitBox(m_width, m_height, 0x4488FF, 1, 100); 
    sprite->SetRelativeLocation({ -m_width * 0.5f, -m_height * 0.5f });
    sprite->SetParentComponent(GetRootComponent());
    AddComponent(std::move(sprite));

    M_LOG("[SlowFloor] Spawned. size=({}, {}), strength={}", m_width, m_height, m_slowStrength);
}


void ASlowFloor::SetSize(float width, float height)
{
    m_width = width;
    m_height = height;
}


void ASlowFloor::BeginOverlap(AActor* OtherActor)
{
    if (!OtherActor) return;

    auto movements = OtherActor->GetComponents<MMovementComponent>();
    for (auto* move : movements)
    {
        m_overlappingMovements.insert(move);
        M_LOG("[SlowFloor] BeginOverlap: {}", OtherActor->GetActorClassName());
    }
}

void ASlowFloor::EndOverlap(AActor* OtherActor)
{
    if (!OtherActor) return;

    auto movements = OtherActor->GetComponents<MMovementComponent>();
    for (auto* move : movements)
    {
        m_overlappingMovements.erase(move);
        M_LOG("[SlowFloor] EndOverlap: {}", OtherActor->GetActorClassName());
    }
}

void ASlowFloor::OnUpdate(float DeltaTime)
{
 
    std::vector<MMovementComponent*> toRemove;

    for (auto* move : m_overlappingMovements)
    {

        if (!move || !move->GetOwner() || move->GetOwner()->IsPendingDestroy())
        {
            toRemove.push_back(move);
            continue;
        }


        float decayPerFrame = std::pow(m_slowStrength, DeltaTime * 60.0f);
        FVector2D v = move->GetVelocity();
        move->SetWorldForce(v * decayPerFrame);
    }

    for (auto* dead : toRemove)
    {
        m_overlappingMovements.erase(dead);
    }
}