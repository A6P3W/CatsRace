#include "ClearScene.h"
#include "SpriteComponent.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Objects/GameScene01.h"
#include <KeyboardDevice.h>
#include <DxLib.h>
AClearScene::AClearScene()
{
    // ��ʒ����Ƀe�L�X�g��\��
    auto textComp = std::make_unique<MSpriteComponent>(100, RenderSpace::Screen);
    textComp->SubmitText("GAME CLEAR!", 0xFFFF00, -1, 255);
    // �X�N���[�����W(RenderSpace::Screen)�Ȃ̂ŁA�𑜓x�ɍ��킹�Ē���
    textComp->SetRelativeLocation({ 800.0f, 500.0f });
    AddComponent(std::move(textComp));

    auto subText = std::make_unique<MSpriteComponent>(100, RenderSpace::Screen);
    subText->SubmitText("Press Z to Return", 0xFFFFFF, -1, 255);
    subText->SetRelativeLocation({ 820.0f, 600.0f });
    AddComponent(std::move(subText));
}

void AClearScene::OnUpdate(float DeltaTime)
{
    auto* kb = InputManager::GetInstance().GetDevice<KeyboardDevice>();
    if (kb && kb->GetPressStart(KEY_INPUT_R)) {
		SceneManager::GetInstance().OpenScene<AGameScene01>();
	}
}