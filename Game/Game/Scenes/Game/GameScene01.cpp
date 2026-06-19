#include "GameScene01.h"
#include <string>
#include "ObjectManager.h"
#include "Scenes/Game/Player.h"
#include "Objects/SampleA.h"
#include "ObjectManager.h"
#include "UMath.h"
#include "Log.h"
#include <PlayerController.h>
#include <Pawn.h>
#include "Goal.h"
#include <CircleCollisionComponent.h>
#include <RectangleCollisionComponent.h>
#include "Objects/Items/SpeedUpItem.h"
#include "ResourceManager.h"
#include <TimerHandle.h>
#include <TimerManager.h>
#include "Objects/Items/SpeedDown.h"
#include <SoundManager.h>
#include <LevelSerializer.h>
#include <SceneManager.h>
#include "Scenes/Clear/ClearScene.h"
#include <UIManager.h>
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Core/GI_main.h"
AGameScene01::AGameScene01()
{
}

void AGameScene01::OnUpdate(float DeltaTime)
{
	if (RaceRunning) {
		RaceTime += DeltaTime;
		if (m_MainHUD) {
			m_MainHUD->UpdateTimerText(RaceTime);
		}
	}
}

void AGameScene01::BeginPlay()
{
	//auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
	//LBM->FetchLeaderBoard("b", [](const std::vector<FLeaderBoardEntry>& entries){
	//	for (const auto& entry : entries) {
	//		M_LOG("User: {}, Score: {}", entry.user_id, entry.score);
	//	}
	//	});
	LevelSerializer::Load(GetWorld(), "tt.json");

	SpawnPlayer<APlayer, APlayerController>(FVector2D{ -2800, -1700 }, 0);

	SpawnActor<ASampleA>();
	M_LOG("Default scene initialized", 0);


	m_MainHUD = SpawnActor<WMainHUD>();
	UIManager::GetInstance()->AddWidget(m_MainHUD);


	m_CountDownWidget = SpawnActor<WCountDown>();
	m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
	UIManager::GetInstance()->AddWidget(m_CountDownWidget);

	GetPlayerController()->SetInputMode(EInputMode::UIOnly);

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::RaceCountDown, 1.0f, true, 1.0f);
}

void AGameScene01::RaceFinish()
{
	RaceRunning = false;
	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	gi->ClearTime = RaceTime;
	gi->map_id = "GameScene01";
	SceneManager::GetInstance().OpenScene<AClearScene>();
}

void AGameScene01::RaceCountDown()
{
	m_CountDown--;

	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		m_CountDownWidget->SetCountText("Go!");
		RaceStart();
	}
	else {
		m_CountDownWidget->SetCountText(std::to_string(m_CountDown));
		M_LOG(std::to_string(m_CountDown), 0);
		GetWorld()->GetSoundManager()->PlaySE("soundreality-pop-423717.mp3", false);
	}
}

void AGameScene01::RaceStart()
{
	M_LOG("start", 0);

	RaceRunning = true;

	GetPlayerController()->SetInputMode(EInputMode::GameOnly);
	dynamic_cast<APlayer*>(GetPlayerPawn())->SetCanMove(true);

	GetWorldTimerManager().SetTimer(CountHandle, this, &AGameScene01::ClearCountDown, 1.0f, false, 1.0f);
}

void AGameScene01::ClearCountDown()
{
	UIManager::GetInstance()->RemoveWidget(m_CountDownWidget);
	m_CountDownWidget = nullptr;
}
