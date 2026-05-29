#include "TitleScene.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
ATitleScene::ATitleScene()
{
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