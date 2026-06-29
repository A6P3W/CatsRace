#include "ClearScene.h"

#include <DxLib.h>
#include <KeyboardDevice.h>
#include <Pawn.h>
#include <imgui.h>

#include <algorithm>
#include <string>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Scenes/Game/GameScene01.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Lobby/LobbyPlayerState.h"
#include "Scenes/Title/TitleScene.h"
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
AClearScene::AClearScene() {}

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

  if (gi->multiplayer_results.empty()) {
    GI_main::FMultiplayerResult result;
    result.ConnectionId = 0;
    result.PlayerName = gi->player_name.empty() ? gi->user_id : gi->player_name;
    result.bFinished = true;
    result.FinishTime = gi->ClearTime;
    gi->multiplayer_results.push_back(result);
  }

  for (const auto& result : gi->multiplayer_results) {
    auto* state = GetWorld()->SpawnActor<ALobbyPlayerState>();
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
