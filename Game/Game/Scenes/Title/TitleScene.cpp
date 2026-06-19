#include "TitleScene.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameScene01.h"
#include "ResourceManager.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
#include "Scenes/Title/UI/WTitleHUD.h"
#include "UIManager.h"
#include "Pawn.h"
#include "PlayerController.h"

ATitleScene::ATitleScene()
{
    int handle = ResourceManager::GetInstance().LoadResourceGraph("images/CatsRace.png");
    auto imgSprite = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
    imgSprite->SubmitGraph( handle);
    imgSprite->SetRelativeLocation({ 960.0f, 540.0f }); 
	imgSprite->SetWorldScale(FScale(1.5));
    AddComponent(std::move(imgSprite));
}

void ATitleScene::BeginPlay()
{
	AGameModeBase::BeginPlay();

	// Setup input and PlayerController for UI interaction
	SpawnPlayer<APawn, APlayerController>({ 0, 0 }, 0);
	GetPlayerController()->SetInputMode(EInputMode::UIOnly);

	// Spawn and add Title HUD
	m_TitleHUD = SpawnActor<WTitleHUD>();
	UIManager::GetInstance()->AddWidget(m_TitleHUD);
	UIManager::GetInstance()->SetFocusedWidget(m_TitleHUD);

}