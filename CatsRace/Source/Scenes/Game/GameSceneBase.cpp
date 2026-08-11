#include "GameSceneBase.h"

#include "Actors/HostServerTravelActor.h"

#include <EnhancedInputComponent.h>
#include <NetworkManager.h>
#include <PlayerController.h>
#include <PlayerStart.h>
#include <SceneManager.h>
#include <SoundManager.h>
#include <TimerHandle.h>
#include <TimerManager.h>
#include <UIManager.h>
#include "Objects/Items/HeldSpeedItem.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Ghost/GhostData.h"
#include "Ghost/GhostPlaybackComponent.h"
#include "Ghost/GhostPlayer.h"
#include "Ghost/GhostRecorderComponent.h"
#include "Log.h"
#include "ActorManager.h"
#include "Objects/SampleA.h"
#include "PC_Game.h"
#include "ResourceManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "Services/LeaderBoardManager.h"
#include "UMath.h"
#include "Objects/Items/HeldSpeedItem.h"

AGameSceneBase::AGameSceneBase(
) {
  SetDefaultPawnClass(APlayer::StaticClassName());
  SetDefaultPlayerControllerClass(PC_Game::StaticClassName());
}

void AGameSceneBase::OnUpdate(float DeltaTime) {
  if (ResultTravelDelay >= 0.0f && GetWorld()->IsServer()) {
    ResultTravelDelay -= DeltaTime;
    if (ResultTravelDelay <= 0.0f) {
      TravelToClear();
    }
  }
  if (RaceRunning) {
    RaceTime += DeltaTime;
    if (m_GhostPlayer && m_GhostPlayer->GetPlaybackComponent()) {
      m_GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
    }
  }

}

void AGameSceneBase::BeginPlay() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    gi->ClearTime = -1.0f;
  }

  AGameModeBase::BeginPlay();

  AHostServerTravelActor::SpawnForListenServer(*GetWorld());


   if (GetWorld()->IsServer()) {
    std::vector<FVector2D> savedLocations;

    for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
      if (auto* editorItem = dynamic_cast<AHeldSpeedItem*>(actorPtr.get())) {
        savedLocations.push_back(editorItem->GetActorLocation());
        editorItem->Destroy();
      }
    }

    for (const auto& loc : savedLocations) {
      SpawnHeldSpeedItemWithRespawn(loc);  // コールバック付きで再スポーン
    }
  }

  SpawnActor<ASampleA>();
  LoadTopGhost();
  M_LOG("Game scene initialized: {}", MapId);

  if (GetWorld()->IsServer()) {
    BeginTravelWait();
  }
}

void AGameSceneBase::OnAllClientsTravelReady() {
  GetWorldTimerManager().SetTimer(
      CountHandle, this, &AGameSceneBase::RaceCountDown, 1.0f, true, 1.0f
  );
}

void AGameSceneBase::OnPlayerSpawned(
    APlayerController* Controller, APawn* Pawn, FNetworkConnectionId ConnectionId
) {
  APlayer* player = dynamic_cast<APlayer*>(Pawn);
  if (!player) return;

  // ゲーム固有のプレイヤー初期化設定
  player->SetCanMove(RaceRunning);

  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    player->SetRotateCamera(gi->bRotateCamera);
  }

  if (ConnectionId == 0) {
    m_GhostRecorder = NewObject<MGhostRecorderComponent>(player);
    m_GhostRecorder->RegisterComponent();
  }

  // 参加リザルト登録
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    const auto playerResult = std::find_if(
        gi->multiplayer_results.begin(),
        gi->multiplayer_results.end(),
        [ConnectionId](const GI_main::FMultiplayerResult& result) {
          return result.ConnectionId == ConnectionId;
        }
    );
    player->SetPlayerName(
        playerResult != gi->multiplayer_results.end()
            ? playerResult->PlayerName
            : ("Player " + std::to_string(ConnectionId + 1))
    );

    auto existing = std::find_if(
        gi->multiplayer_results.begin(),
        gi->multiplayer_results.end(),
        [ConnectionId](const GI_main::FMultiplayerResult& result) {
          return result.ConnectionId == ConnectionId;
        }
    );
    if (existing == gi->multiplayer_results.end()) {
      GI_main::FMultiplayerResult result;
      result.ConnectionId = ConnectionId;
      result.PlayerName = ConnectionId == 0 && !gi->player_name.empty()
                              ? gi->player_name
                              : ("Player " + std::to_string(ConnectionId + 1));
      gi->multiplayer_results.push_back(result);
    }
  }
}

void AGameSceneBase::LoadTopGhost() {
  auto* lbm = GetWorld()->SpawnActor<LeaderBoardManager>();
  const std::string mapId = MapId;
  lbm->FetchLeaderBoard(
      mapId, [this, lbm, mapId](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
        if (!bSuccess || entries.empty()) {
          M_LOG("Top ghost skipped: leaderboard is empty or unavailable");
          return;
        }

        const std::string topUserId = entries.front().user_id;
        lbm->FetchGhostData(
            mapId,
            {topUserId},
            [this, topUserId](
                bool bGhostSuccess,
                const std::unordered_map<std::string, std::string>& ghostDataById
            ) {
              auto ghostDataIt = ghostDataById.find(topUserId);
              if (!bGhostSuccess || ghostDataIt == ghostDataById.end() ||
                  ghostDataIt->second.empty()) {
                M_LOG("Top ghost skipped: ghost data is empty for {}", topUserId);
                return;
              }

              auto frames = GhostDataSerializer::Deserialize(ghostDataIt->second);
              if (frames.empty()) {
                M_LOG("Top ghost skipped: failed to parse ghost data for {}", topUserId);
                return;
              }

              if (!m_GhostPlayer) {
                m_GhostPlayer = GetWorld()->SpawnActor<AGhostPlayer>();
              }
              m_GhostPlayer->SetUserId(topUserId);
              m_GhostPlayer->SetGhostData(frames);
              if (m_GhostPlayer->GetPlaybackComponent()) {
                m_GhostPlayer->GetPlaybackComponent()->UpdatePlayback(RaceTime);
              }
              M_LOG("Top ghost loaded: {} frames from {}", frames.size(), topUserId);
            }
        );
      }
  );
}

void AGameSceneBase::RaceFinish() {
  if (auto* player = dynamic_cast<APlayer*>(GetPlayerPawn())) {
    NotifyPlayerFinished(player);
  }
}

APlayerController* AGameSceneBase::OnClientConnected(FNetworkConnectionId ConnectionId) {
  return AGameModeBase::OnClientConnected(ConnectionId);
}

void AGameSceneBase::OnClientDisconnected(FNetworkConnectionId ConnectionId) {
  if (GetWorld() && GetWorld()->GetActorManager()) {
    for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
      auto* player = dynamic_cast<APlayer*>(actorPtr.get());
      if (player && player->OwnerConnectionId == ConnectionId) {
        player->Destroy();
      }
      auto* pc = dynamic_cast<APlayerController*>(actorPtr.get());
      if (pc && pc->OwnerConnectionId == ConnectionId) {
        pc->Destroy();
      }
    }
  }
  AGameModeBase::OnClientDisconnected(ConnectionId);
}

void AGameSceneBase::NotifyPlayerFinished(APlayer* Player) {
  if (!Player || !GetWorld()->IsServer() || bResultTravelRequested) {
    return;
  }

  SaveResult(Player->OwnerConnectionId, RaceTime);
  if (Player->OwnerConnectionId == 0 && m_GhostRecorder) {
    m_GhostRecorder->StopRecording();
  }

  if (AreAllPlayersFinished()) {
    TravelToClear();
    return;
  }

  if (ResultTravelDelay < 0.0f) {
    ResultTravelDelay = 30.0f;
  }
}

void AGameSceneBase::SaveResult(FNetworkConnectionId ConnectionId, float FinishTime) {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    gi->ClearTime = FinishTime;
    gi->map_id = MapId;
    gi->LastGhostData =
        (ConnectionId == 0 && m_GhostRecorder) ? m_GhostRecorder->GetSerializedData() : "";
    auto existing = std::find_if(
        gi->multiplayer_results.begin(),
        gi->multiplayer_results.end(),
        [ConnectionId](const GI_main::FMultiplayerResult& result) {
          return result.ConnectionId == ConnectionId;
        }
    );
    if (existing == gi->multiplayer_results.end()) {
      GI_main::FMultiplayerResult result;
      result.ConnectionId = ConnectionId;
      result.PlayerName = ConnectionId == 0 && !gi->player_name.empty()
                              ? gi->player_name
                              : ("Player " + std::to_string(ConnectionId + 1));
      result.bFinished = true;
      result.FinishTime = FinishTime;
      gi->multiplayer_results.push_back(result);
    } else if (!existing->bFinished) {
      existing->bFinished = true;
      existing->FinishTime = FinishTime;
    }
  }
}

bool AGameSceneBase::AreAllPlayersFinished() const {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    if (gi->multiplayer_results.empty()) {
      return false;
    }
    return std::all_of(
        gi->multiplayer_results.begin(),
        gi->multiplayer_results.end(),
        [](const GI_main::FMultiplayerResult& result) { return result.bFinished; }
    );
  }
  return false;
}

void AGameSceneBase::TravelToClear() {
  if (bResultTravelRequested || !GetWorld()->IsServer()) {
    return;
  }
  bResultTravelRequested = true;
  RaceRunning = false;
  GetWorld()->ServerTravel(GameSceneIds::Clear);
}

void AGameSceneBase::RaceCountDown() {
  m_CountDown--;

  if (m_CountDown <= 0) {
    GetWorldTimerManager().ClearTimer(CountHandle);
    RaceStart();
  }
}

void AGameSceneBase::RaceStart() {
  M_LOG("start", 0);

  RaceRunning = true;
  if (m_GhostRecorder) {
    m_GhostRecorder->StartRecording();
  }

  if (GetWorld() && GetWorld()->GetActorManager()) {
    for (const auto& actorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
      if (auto* player = dynamic_cast<APlayer*>(actorPtr.get())) {
        player->SetCanMove(true);
      }
    }
  }
}

void AGameSceneBase::RestartGame() { OpenCurrentScene(); }

void AGameSceneBase::ReturnToLobby() {
  SceneManager::GetInstance().OpenLevelById(GameSceneIds::Menu);
}
void AGameSceneBase::SpawnHeldSpeedItemWithRespawn(FVector2D location) {
  auto* item = GetWorld()->SpawnActor<AHeldSpeedItem>(location);
  if (!item) return;

  item->SetOnPickedUp([this, location]() {
    // タイマーハンドルをメンバに積んで SetTimer のメンバ関数版で呼ぶ
    m_pendingRespawnLocations.push_back(location);
    FTimerHandle handle;
    GetWorldTimerManager().SetTimer(handle, this, &AGameSceneBase::RespawnNextItem, 3.0f, false);
    m_itemRespawnHandles.push_back(handle);
  });
}

void AGameSceneBase::RespawnNextItem() {
  if (m_pendingRespawnLocations.empty()) return;
  FVector2D loc = m_pendingRespawnLocations.front();
  m_pendingRespawnLocations.erase(m_pendingRespawnLocations.begin());
  SpawnHeldSpeedItemWithRespawn(loc);
}
