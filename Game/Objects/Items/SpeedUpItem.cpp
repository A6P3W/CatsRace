#include "SpeedUpItem.h"
#include "CircleCollisionComponent.h"
#include "Objects/Player.h"
#include "Utils/Log.h"
#include "MovementComponent.h"
#include <SpriteComponent.h>
SpeedUpItem::SpeedUpItem()
{
	auto collision = std::make_unique<MCircleCollisionComponent>(500.0f);
	collision->SetParentComponent(GetRootComponent());
	collision->SetCollisionType(ECollisionType::Overlap);
	AddComponent(std::move(collision));

	auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
	sprite->SubmitCircle(500.0f, 0x00FF00, 1, 128);
	sprite->SetParentComponent(GetRootComponent());
	AddComponent(std::move(sprite));
}

void SpeedUpItem::BeginOverlap(AActor* OtherActor)
{
	if (dynamic_cast<APlayer*>(OtherActor)) {
		OtherActor->GetComponents<MMovementComponent>()[0]->AddLocalForce({ 0, -250.0f });
		M_LOG("Speed Up!");
	}
}
