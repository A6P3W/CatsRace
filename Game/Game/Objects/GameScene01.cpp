#include "GameScene01.h"
#include <string>
#include "ObjectManager.h"
#include "Objects/Player.h"
#include "Objects/SampleA.h"
#include "Objects/Map.h"
#include "ObjectManager.h"
#include <Utils/Umath.h>
#include <Utils/Log.h>
#include <SpriteComponent.h>
#include <PlayerController.h>
#include <Pawn.h>
#include "Goal.h"
#include <CircleCollisionComponent.h>
#include <RectangleCollisionComponent.h>
#include "UserTimer.h"
#include "Objects/Items/SpeedUpItem.h"
#include "ResourceManager.h"
#include <TimerHandle.h>
#include <TimerManager.h>
AGameScene01::AGameScene01() {
	int imgHandle = ResourceManager::GetInstance().LoadResourceGraph("images/Sprite-0004.png");
	auto imgSprite = std::make_unique<MSpriteComponent>(-5000, RenderSpace::World);
	imgSprite->SubmitGraph( imgHandle);
	imgSprite->SetWorldScale(50.0);
	imgSprite->SetRelativeLocation({ 500.0f, 300.0f });
	AddComponent(std::move(imgSprite));

	auto sprite = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite->SubmitCircle(300.0f, 0xFF0000, 1, 128);
	AddComponent(std::move(sprite));
	auto sprite2 = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite2->SubmitText("center", 0xFF0000, -1, 255);
	AddComponent(std::move(sprite2));

	auto c = std::make_unique<MRectangleCollisionComponent>();
	c->SetCollisionType(ECollisionType::Block);
	c->SetWorldScale(90.0f);
	c->SetWorldLocation({ -4500,-2393 });
	AddComponent(std::move(c));

	auto d = std::make_unique<MRectangleCollisionComponent>();
	d->SetCollisionType(ECollisionType::Block);
	d->SetWorldScale(90.0f);
	d->SetWorldLocation({ -4450,-19550 });
	AddComponent(std::move(d));

	auto e = std::make_unique<MRectangleCollisionComponent>();
	e->SetCollisionType(ECollisionType::Block);
	e->SetWorldScale(90.0f);
	e->SetWorldLocation({ -11950,-10700 });
	AddComponent(std::move(e));

	auto a = std::make_unique<MRectangleCollisionComponent>();
	a->SetCollisionType(ECollisionType::Block);
	a->SetWorldScale(180.0f);
	a->SetWorldLocation({ 11400,0 });
	AddComponent(std::move(a));

	auto b = std::make_unique<MRectangleCollisionComponent>();
	b->SetCollisionType(ECollisionType::Block);
	b->SetWorldScale(180.0f);
	b->SetWorldLocation({ 11450,-15000 });
	AddComponent(std::move(b));

	auto i = std::make_unique<MRectangleCollisionComponent>();
	i->SetCollisionType(ECollisionType::Block);
	i->SetWorldScale(90.0f);
	i->SetWorldLocation({ 0,-28500 });
	AddComponent(std::move(i));

	auto j = std::make_unique<MRectangleCollisionComponent>();
	j->SetCollisionType(ECollisionType::Block);
	j->SetWorldScale(90.0f);
	j->SetWorldLocation({ 0, 5500 });
	AddComponent(std::move(j));

	auto f = std::make_unique<MRectangleCollisionComponent>();
	f->SetCollisionType(ECollisionType::Block);
	f->SetWorldScale(34.2f);
	f->SetWorldLocation({ 800,-10970 });
	AddComponent(std::move(f));


	auto g = std::make_unique<MRectangleCollisionComponent>();
	g->SetCollisionType(ECollisionType::Block);
	g->SetWorldScale(34.2f);
	g->SetWorldLocation({ -1800,-10970 });
	AddComponent(std::move(g));

	auto h = std::make_unique<MRectangleCollisionComponent>();
	h->SetCollisionType(ECollisionType::Block);
	h->SetWorldScale(34.2f);
	h->SetWorldLocation({ -3330,-10970 });
	AddComponent(std::move(h));


	auto countSprite = std::make_unique<MSpriteComponent>(110, RenderSpace::World);
	m_CountDownSprite = countSprite.get();
	m_CountDownSprite->SetRelativeLocation({ 0.0f, -2000.0f });
	AddComponent(std::move(countSprite));
}

void AGameScene01::OnUpdate(float DeltaTime)
{
}

void AGameScene01::BeginPlay()
{
	SpawnActor<AMap>(FVector2D::ZeroVector, 0.0f);

	SpawnPlayer<APlayer, APlayerController>(FVector2D{ 1111, 0 }, 0);

	SpawnActor<ASampleA>({ 0,0 }, { 0 });
	M_LOG("Default scene initialized", 0);

	SpawnActor<AGoalActor>(FVector2D{ 1000.0f, -21150.0f }, 0.0f);

	SpawnActor<SpeedUpItem>(FVector2D{ 0,400 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::RaceCountDown, 1.0f, true, 0);
}

void AGameScene01::RaceCountDown()
{
	auto text = ResourceManager::GetInstance().GetFont(240, 1);
	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		RaceStart();
		
		m_CountDownSprite->SubmitText("Go!", 0xFFFFFF, text, 255);
	}
	else {
		m_CountDownSprite->SubmitText(std::to_string(m_CountDown), 0xFFFFFF, text, 255);
		M_LOG(std::to_string(m_CountDown), 0);
		m_CountDown--;
	}
}


void AGameScene01::RaceStart()
{
	M_LOG("start", 0);
	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<UserTimer>(FVector2D{ 0,0 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);


	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<SpeedUpItem>(FVector2D{ 1111,  -3000 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);

	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<SpeedUpItem>(FVector2D{ -1111,  -9000 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);

	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<SpeedUpItem>(FVector2D{ -7000,  -10000 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);

	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<SpeedUpItem>(FVector2D{ 2000,  -15000 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);
}
void AGameScene01::ClearCountDownSprite()
{
	m_CountDownSprite->DestroyComponent();
}

