#pragma once
#include "PlayerController.h"

class EditorMode;
class MEnhancedInputComponent;
class PC_Game : public APlayerController
{
public:
	PC_Game();

	void SetupPlayerInputComponent(MEnhancedInputComponent* PlayerInputComponent) override;
	void SetupInputMappings() override;
private:
};
