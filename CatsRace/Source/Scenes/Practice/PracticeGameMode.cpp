#include "PracticeGameMode.h"

#include "Gameplay/Race/Player/PC_Game.h"
#include "Gameplay/Race/Player/Player.h"

REGISTER_GAME_MODE(APracticeGameMode)

APracticeGameMode::APracticeGameMode() {
  SetDefaultPawnClass(APlayer::StaticClassName());
  SetDefaultPlayerControllerClass(PC_Game::StaticClassName());
}
