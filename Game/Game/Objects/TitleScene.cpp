#include "TitleScene.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include "ResourceManager.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
ATitleScene::ATitleScene()
{
    int handle = ResourceManager::GetInstance().LoadResourceGraph("images/CatsRace.png");
    auto imgSprite = std::make_unique<MSpriteComponent>(0, RenderSpace::Screen);
    imgSprite->SubmitGraph(0.8, handle);
    imgSprite->SetRelativeLocation({ 480.0f, 270.0f }); 
    AddComponent(std::move(imgSprite));

    auto textComp = std::make_unique<MSpriteComponent>(100, RenderSpace::Screen);
    textComp->SubmitText("Press R to Start", 0xFFFF00, -1, 255);
    textComp->SetRelativeLocation({ 800.0f, 500.0f });
    AddComponent(std::move(textComp));
}

void ATitleScene::OnUpdate(float DeltaTime)
{
    auto* kb = InputManager::GetInstance().GetDevice<KeyboardDevice>();
    if (kb && kb->GetPressStart(KEY_INPUT_R)) {
        SceneManager::GetInstance().OpenScene<AGameScene01>();
    }
}