#include "ClearScene.h"

#include <KeyboardDevice.h>
#include <Pawn.h>

#include <algorithm>
#include <string>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Services/LeaderBoardManager.h"
#include "SpriteComponent.h"
#include "UI/WClearHUD.h"
#include "UIManager.h"
#include "World.h"

REGISTER_GAME_MODE(AClearScene)
AClearScene::AClearScene() { SetDefaultPlayerControllerClass("PC_Clear"); }

void AClearScene::BeginPlay() {
  AGameModeBase::BeginPlay();

  if (GetWorld()->IsServer()) {
    SpawnResultStatesFromGameInstance();
  }
}

void AClearScene::SpawnResultStatesFromGameInstance() {
  auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance());
  if (!gi) {
    return;
  }

  for (const auto& result : gi->multiplayer_results) {
    auto* state = GetWorld()->SpawnActor<ALobbyPlayerState>();
    if (!state) {
      continue;
    }

    state->OwnerConnectionId = result.ConnectionId;
    state->bReplicates = true;
    state->bHasAuthority = true;
    state->bIsLocallyControlled = result.ConnectionId == 0;
    state->SetPlayerName(result.PlayerName);
    state->SetFinishResult(result.bFinished, result.FinishTime);
  }
}

void AClearScene::OnUpdate(float DeltaTime) {
  if (GetWorld()->IsServer() && GetWorld()->GetActorManager()) {
    for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
      if (auto* player = dynamic_cast<APlayer*>(actorPtr.get())) {
        player->Destroy();
      }
    }
  }

  if (!GetWorld()->IsServer() || GetWorld()->IsStandalone() || bReturnToLobbyRequested) {
    return;
  }

  ReturnToLobbyRemaining -= DeltaTime;
  if (ReturnToLobbyRemaining > 0.0f) {
    return;
  }

  bReturnToLobbyRequested = true;
  GetWorld()->ServerTravel(GameSceneIds::Lobby);
}
