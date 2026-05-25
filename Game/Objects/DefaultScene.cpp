#include "DefaultScene.h"
#include <string>
#include "ObjectManager.h"
#include "Objects/Player.h"
#include "Objects/SampleA.h"
#include "Objects/Map.h"
#include "ObjectManager.h"
#include <Utils/Umath.h>
#include <Utils/Log.h>
#include <SpriteComponent.h>
#include "CSVMap.h"
#include <PlayerController.h>
#include <Pawn.h>
#include "Goal.h"
#include <CircleCollisionComponent.h>
#include <RectangleCollisionComponent.h>
#include "UserTimer.h"
#include "Objects/Items/SpeedUpItem.h"
#include "ResourceManager.h" 
ADefaultScene::ADefaultScene() {
	ObjectManager::GetInstance().SpawnObject<AMap>(FVector2D::ZeroVector, 0.0f);

	SpawnPlayer<APlayer, APlayerController>(FVector2D{ 1111, 0 }, 0);

	ObjectManager::GetInstance().SpawnObject<ASampleA>({ 0,0 }, { 0 });
	M_LOG("Default scene initialized", 0);

	int imgHandle = ResourceManager::GetInstance().LoadResourceGraph("images/Sprite-0004.png"); 
	auto imgSprite = std::make_unique<MSpriteComponent>(-5000, RenderSpace::World);
	imgSprite->SubmitGraph(1.0, imgHandle, 255);
	imgSprite->SetScale(50.0);
	imgSprite->SetRelativeLocation({ 500.0f, 300.0f }); 
	AddComponent(std::move(imgSprite));

	auto sprite = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite->SubmitCircle(300.0f, 0xFF0000, 1, 128);
	AddComponent(std::move(sprite));
	auto sprite2 = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite2->SubmitText("center", 0xFF0000, -1, 255);
	AddComponent(std::move(sprite2));

	ObjectManager::GetInstance().SpawnObject<AGoalActor>(FVector2D{ 1000.0f, -21000.0f }, 0.0f);


	ObjectManager::GetInstance().SpawnObject<ACSVMap>(FVector2D{0,0 }, FRotator{ 0 });


	ObjectManager::GetInstance().SpawnObject<UserTimer>(FVector2D{ 0,0 }, FRotator{ 0 });


	auto c = std::make_unique<MRectangleCollisionComponent>();
	c->SetCollisionType(ECollisionType::Block);
	c->SetScale(90.0f);
	c->SetWorldLocation({ -4500,-2393 });
	AddComponent(std::move(c));

	ObjectManager::GetInstance().SpawnObject<SpeedUpItem>(FVector2D{ 0,400 }, FRotator{ 0 });
	auto d = std::make_unique<MRectangleCollisionComponent>();
	d->SetCollisionType(ECollisionType::Block);
	d->SetScale(90.0f);
	d->SetWorldLocation({ -4450,-19550});
	AddComponent(std::move(d));

	auto e = std::make_unique<MRectangleCollisionComponent>();
	e->SetCollisionType(ECollisionType::Block);
	e->SetScale(90.0f);
	e->SetWorldLocation({ -11950,-10500 });
	AddComponent(std::move(e));

	auto a = std::make_unique<MRectangleCollisionComponent>();
	a->SetCollisionType(ECollisionType::Block);
	a->SetScale(180.0f);
	a->SetWorldLocation({ 11400,0 });
	AddComponent(std::move(a));

	auto b = std::make_unique<MRectangleCollisionComponent>();
	b->SetCollisionType(ECollisionType::Block);
	b->SetScale(180.0f);
	b->SetWorldLocation({ 11450,-15000 });
	AddComponent(std::move(b));

	auto i = std::make_unique<MRectangleCollisionComponent>();
	i->SetCollisionType(ECollisionType::Block);
	i->SetScale(90.0f);
	i->SetWorldLocation({ 0,-28500 });
	AddComponent(std::move(i));

	auto j = std::make_unique<MRectangleCollisionComponent>();
	j->SetCollisionType(ECollisionType::Block);
	j->SetScale(90.0f);
	j->SetWorldLocation({ 0, 5500 });
	AddComponent(std::move(j));

	auto f = std::make_unique<MRectangleCollisionComponent>();
	f->SetCollisionType(ECollisionType::Block);
	f->SetScale(34.2f);
	f->SetWorldLocation({800,-10970 });
	AddComponent(std::move(f));


	auto g = std::make_unique<MRectangleCollisionComponent>();
	g->SetCollisionType(ECollisionType::Block);
	g->SetScale(34.2f);
	g->SetWorldLocation({ -1800,-10970 });
	AddComponent(std::move(g));

	auto h = std::make_unique<MRectangleCollisionComponent>();
	h->SetCollisionType(ECollisionType::Block);
	h->SetScale(34.2f);
	h->SetWorldLocation({ -3330,-10970 });
	AddComponent(std::move(h));

}

void ADefaultScene::OnUpdate(float DeltaTime)
{
}
