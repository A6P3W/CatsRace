#include "ClearScene.h"

#include <DxLib.h>
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
#include "UI/WNameInputDialog.h"
#include "UI/WNameSelectDialog.h"
#include "UI/WOverwriteConfirmDialog.h"
#include "UI/WPostGameDialog.h"
#include "UIManager.h"
#include "World.h"

REGISTER_GAME_MODE(AClearScene)
AClearScene::AClearScene() { DefaultPlayerControllerClass = "PC_Clear"; }

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
  (void)DeltaTime;
  if (GetWorld()->IsServer() && GetWorld()->GetObjectManager()) {
    for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
      if (auto* player = dynamic_cast<APlayer*>(actorPtr.get())) {
        player->Destroy();
      }
    }
  }
}
