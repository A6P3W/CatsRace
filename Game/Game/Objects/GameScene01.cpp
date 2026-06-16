#include "GameScene01.h"
#include <string>
#include "ObjectManager.h"
#include "Objects/Player.h"
#include "Objects/SampleA.h"
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
#include <SceneManager.h>
#include "Objects/ClearScene.h"
AGameScene01::AGameScene01() {
	auto countSprite = std::make_unique<MSpriteComponent>(110, RenderSpace::World);
	m_CountDownSprite = countSprite.get();
	m_CountDownSprite->SetRelativeLocation({ -2900.0f, -2200.0f });
	AddComponent(std::move(countSprite));
}

void AGameScene01::OnUpdate(float DeltaTime)
{
	if (RaceRunning) {
		RaceTime += DeltaTime;
	}
}

void AGameScene01::BeginPlay()
{
	LevelSerializer::Load(GetWorld(), "tt.json");

	SpawnPlayer<APlayer, APlayerController>(FVector2D{ -2800, -1700 }, 0);
	
	SpawnActor<ASampleA>();
	M_LOG("Default scene initialized", 0);

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::RaceCountDown, 1.0f, true, 0);
}

void AGameScene01::RaceFinish()
{
	RaceRunning = false;
	SceneManager::GetInstance().OpenScene<AClearScene>();
	
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
	RaceRunning = true;

	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);
	SpawnActor<UserTimer>(FVector2D{ 0,0 }, FRotator{ 0 });
	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDownSprite, 1.0f, false, 2.0f);
}
void AGameScene01::ClearCountDownSprite()
{
	m_CountDownSprite->DestroyComponent();
}

