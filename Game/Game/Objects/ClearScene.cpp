#include "ClearScene.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
#include "GI_main.h"
#include "UI/WClearHUD.h"
#include "LeaderBoardManager.h"
#include "UIManager.h"
#include "World.h"
#include "SpriteComponent.h"
#include <string>

AClearScene::AClearScene()
{
	auto subText = std::make_unique<MSpriteComponent>(100, RenderSpace::Screen);
	subText->SubmitText("Press R to Return", 0xFFFFFF, -1, 255);
	subText->SetRelativeLocation({ 820.0f, 800.0f });
	AddComponent(std::move(subText));
}

void AClearScene::BeginPlay()
{
	AGameModeBase::BeginPlay();

	auto gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
	float clearTime = gi->ClearTime;
	std::string map_id = gi->map_id;

	m_ClearHUD = GetWorld()->SpawnActor<WClearHUD>();
	UIManager::GetInstance()->AddWidget(m_ClearHUD);

	m_ClearHUD->SetClearTime(clearTime);

	auto* LBM = GetWorld()->SpawnActor<LeaderBoardManager>();
	LBM->PostScore(map_id, [this, LBM, map_id](bool bSuccess) {
		if (!bSuccess) {
			m_ClearHUD->SetErrorText("Failed to Post Score");
			return;
		}
		LBM->FetchLeaderBoard(map_id, [this](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
			if (!bSuccess) {
				m_ClearHUD->SetErrorText("Failed to Fetch LeaderBoard");
				return;
			}
			if (m_ClearHUD) {
				m_ClearHUD->SetLeaderBoard(entries);
			}
			});
		});
}

void AClearScene::OnUpdate(float DeltaTime)
{
	auto* kb = InputManager::GetInstance().GetDevice<KeyboardDevice>();
	if (kb && kb->GetPressStart(KEY_INPUT_R)) {
		SceneManager::GetInstance().OpenScene<AGameScene01>();
	}
}