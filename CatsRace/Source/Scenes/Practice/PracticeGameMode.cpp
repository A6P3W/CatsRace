#include "PracticeGameMode.h"

#include "Scenes/Game/PC_Game.h"
#include "Scenes/Game/Player.h"

REGISTER_GAME_MODE(APracticeGameMode)

APracticeGameMode::APracticeGameMode() {
  SetDefaultPawnClass(APlayer::StaticClassName());
  SetDefaultPlayerControllerClass(PC_Game::StaticClassName());
}