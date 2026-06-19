#include "PC_Game.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "KeyboardDevice.h"
#include "GamePadDevice.h"
#include "EnhancedInputComponent.h"
#include "World.h"
#include <DxLib.h>

PC_Game::PC_Game()
{}

void PC_Game::SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent)
{
	APlayerController::SetupPlayerInputComponent(PlayerInputComponent);
}

void PC_Game::SetupInputMappings()
{
	APlayerController::SetupInputMappings();

	auto& IM = InputManager::GetInstance();
	auto* kb = IM.GetDevice<KeyboardDevice>();
	auto* pad = IM.GetDevice<GamepadDevice>();
	auto* Mapper = GetInputMapper();

	Mapper->RemoveMapping(InputActionLower::MoveY);
	Mapper->RemoveMapping(InputAction::Interact);

	if (kb) {
		Mapper->AddMapping(InputActionLower::MoveY, kb, KEY_INPUT_W, "", 1.0f);
		Mapper->AddMapping(InputActionLower::MoveY, kb, KEY_INPUT_S, "", -1.0f);
		Mapper->AddMapping(InputAction::Interact, kb, KEY_INPUT_F);
		Mapper->AddMapping("DRIFT", kb, KEY_INPUT_SPACE);
	}
	if (pad) {
		// ゲームプレイ中の MoveY には左スティックをバインドせず、トリガーのみで操作させる
		Mapper->AddAxisMapping(InputActionLower::MoveY, pad, static_cast<int>(AxisID::RightTrigger), 1.0f);
		Mapper->AddAxisMapping(InputActionLower::MoveY, pad, static_cast<int>(AxisID::LeftTrigger), -1.0f);

		Mapper->AddMapping(InputAction::Interact, pad, PAD_INPUT_4);
		Mapper->AddMapping("DRIFT", pad, PAD_INPUT_1);
	}
}