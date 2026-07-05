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
