#include "GameScene01.h"
#include <string>
#include "ObjectManager.h"
#include "Objects/Player.h"
#include "Objects/SampleA.h"
#include "Objects/Map.h"
#include "ObjectManager.h"
#include "UMath.h"
#include "Log.h"
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
#include "Objects/Items/SpeedDown.h"
#include <SoundManager.h>
#include <LevelSerializer.h>
AGameScene01::AGameScene01() {
	int imgHandle = ResourceManager::GetInstance().LoadResourceGraph("images/stage1.png");
	auto imgSprite = std::make_unique<MSpriteComponent>(-5000, RenderSpace::World);
	imgSprite->SubmitGraph( imgHandle);
	imgSprite->SetWorldScale(FScale(50.0f));
	imgSprite->SetRelativeLocation({ 500.0f, 300.0f });
	AddComponent(std::move(imgSprite));

	auto sprite = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite->SubmitCircle(300.0f, 0xFF0000, 1, 128);
	AddComponent(std::move(sprite));
	auto sprite2 = std::make_unique<MSpriteComponent>(-10000, RenderSpace::World);
	sprite2->SubmitText("center", 0xFF0000, -1, 255);
	AddComponent(std::move(sprite2));

	

	auto countSprite = std::make_unique<MSpriteComponent>(110, RenderSpace::World);
	m_CountDownSprite = countSprite.get();
	m_CountDownSprite->SetRelativeLocation({ -2900.0f, -2200.0f });
	AddComponent(std::move(countSprite));
}

void AGameScene01::OnUpdate(float DeltaTime)
{
}

void AGameScene01::BeginPlay()
{
	LevelSerializer::Load(GetWorld(), "tt.json");
	SpawnActor<AMap>();

	SpawnPlayer<APlayer, APlayerController>(FVector2D{ -2800, -1700 }, 0);

	SpawnActor<ASlowFloor>(FVector2D{ 500.0f, -8000.0f });
	
	SpawnActor<ASampleA>();
	M_LOG("Default scene initialized", 0);

	SpawnActor<AGoalActor>(FVector2D{ 1000.0f, -21150.0f });

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
		GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);

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

