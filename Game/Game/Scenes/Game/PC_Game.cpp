#include "PC_Game.h"
#include "InputManager.h"
#include "InputMapper.h"
#include "KeyboardDevice.h"
#include "EnhancedInputComponent.h"
#include "World.h"
#include<DxLib.h>
PC_Game::PC_Game()
{}

void PC_Game::SetupPlayerInputComponent(MEnhancedInputComponent * PlayerInputComponent)
{}

void PC_Game::SetupInputMappings()
{
	APlayerController::SetupInputMappings();

	auto& IM = InputManager::GetInstance();
	auto* kb = IM.GetDevice<KeyboardDevice>();

	auto* Mapper = GetInputMapper();

	if (kb) {
		Mapper->AddMapping("DRIFT", kb, KEY_INPUT_SPACE);
	}
}