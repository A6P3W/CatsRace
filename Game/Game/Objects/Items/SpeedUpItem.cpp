#include "SpeedUpItem.h"
#include "CircleCollisionComponent.h"
#include "Objects/Player.h"
#include "Log.h"
#include "MovementComponent.h"
#include <SpriteComponent.h>
#include "SoundComponent.h"
REGISTER_ACTOR(SpeedUpItem);
SpeedUpItem::SpeedUpItem()
{
	auto collision = std::make_unique<MCircleCollisionComponent>(100.0f);
	collision->SetParentComponent(GetRootComponent());
	collision->SetCollisionType(ECollisionType::Overlap);
	AddComponent(std::move(collision));

	auto sprite = std::make_unique<MSpriteComponent>(0, RenderSpace::World);
	sprite->SubmitCircle(100.0f, 0x00FF00, 1, 128);
	sprite->SetParentComponent(GetRootComponent());
	AddComponent(std::move(sprite));

	auto sound = std::make_unique<MSoundComponent>();
	m_sound = sound.get();
	AddComponent(std::move(sound));
}

void SpeedUpItem::BeginOverlap(AActor* OtherActor)
{
	if (auto* player = dynamic_cast<APlayer*>(OtherActor)) {
		player->GetComponents<MMovementComponent>()[0]->AddLocalForce({ 0, -25.0f });
		player->ApplyFOVEffect(0.7f, 2.0f);
		if (m_sound) m_sound->PlaySE("images/cat2d.mp3", false); // お好みのSEパスに変更
		M_LOG("Speed Up!");
	}
}