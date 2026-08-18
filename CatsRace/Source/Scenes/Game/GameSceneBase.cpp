#include "GameSceneBase.h"

#include <EnhancedInputComponent.h>
#include <NetBuffer.h>
#include <NetPacketType.h>
#include <NetworkManager.h>
#include <PlayerController.h>
#include <PlayerStart.h>
#include <SceneManager.h>
#include <SoundManager.h>
#include <TimerHandle.h>
#include <TimerManager.h>
#include <UIManager.h>

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ActorManager.h"
#include "Actors/HostServerTravelActor.h"
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Ghost/GhostRecorderComponent.h"
#include "Log.h"
#include "Objects/Items/HeldSpeedItem.h"
#include "Objects/SampleA.h"
#include "PC_Game.h"
#include "ResourceManager.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Game/Player.h"
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "UMath.h"

AGameSceneBase::AGameSceneBase() {
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
  if (!RaceRunning && bHasRaceStartTime &&
      NetworkManager::GetInstance().GetEstimatedServerTime() >= RaceStartServerTime) {
    RaceStart();
  } else if (RaceRunning) {
    RaceTime += DeltaTime;
  }
}

void AGameSceneBase::BeginPlay() {
  if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
    MapId = gi->RaceMapId;
    MapVersion = gi->RaceMapVersion;
    if (MapId.empty()) {
      if (const FMapInfo* Map = FindMapInfo(gi->last_level_path)) {
        MapId = Map->MapId;
        MapVersion = Map->MapVersion;
      }
    }
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
  M_LOG(Log, "Game scene initialized: {}", MapId);

  if (GetWorld()->IsServer()) {
    BeginTravelWait();
  }
}

void AGameSceneBase::OnAllClientsTravelReady() {
  if (bHasRaceStartTime) {
    return;
  }

  RaceStartServerTime = NetworkManager::GetInstance().GetEstimatedServerTime() + 3.0;
  bHasRaceStartTime = true;

  if (!GetWorld() || !GetWorld()->GetActorManager()) {
    return;
  }
  for (const auto& ActorPtr : GetWorld()->GetActorManager()->GetAllActors()) {
    auto* Controller = dynamic_cast<PC_Game*>(ActorPtr.get());
    if (Controller && Controller->OwnerConnectionId == 0) {
      Controller->ReceiveRaceStartTime(RaceStartServerTime);
      break;
    }
  }

  if (NetworkManager::GetInstance().GetConnectedClientCount() > 0) {
    FNetBuffer Buffer;
    Buffer.Write(ENetPacketType::RaceStartTime);
    Buffer.Write(RaceStartServerTime);
    if (!NetworkManager::GetInstance().Broadcast(Buffer, ENetPacketReliability::Reliable)) {
      M_LOG(Warning, "Failed to broadcast race start time: {}", RaceStartServerTime);
    }
  }
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

  if (GetWorld()->IsServer()) {
    auto* Recorder = NewObject<MGhostRecorderComponent>(player);
    Recorder->RegisterComponent();
    GhostRecorders[ConnectionId] = Recorder;
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
    if (playerResult != gi->multiplayer_results.end()) {
      player->SetPlayerName(playerResult->PlayerName);
      player->SetPlayerColorIndex(playerResult->PlayerColorIndex);
    } else {
      player->SetPlayerName("Player " + std::to_string(ConnectionId + 1));
    }

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
    MGhostRecorderComponent* Recorder = nullptr;
    if (const auto Found = GhostRecorders.find(ConnectionId); Found != GhostRecorders.end()) {
      Recorder = Found->second;
      if (Recorder) {
        Recorder->StopRecording();
      }
    }
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
      if (Recorder) {
        result.GhostData = Recorder->GetSerializedData();
        result.GhostRecordedSeconds = Recorder->GetRecordedSeconds();
        result.bIsGhostPartial = Recorder->IsPartialGhost();
      }
      gi->multiplayer_results.push_back(result);
    } else if (!existing->bFinished) {
      existing->bFinished = true;
      existing->FinishTime = FinishTime;
      if (Recorder) {
        existing->GhostData = Recorder->GetSerializedData();
        existing->GhostRecordedSeconds = Recorder->GetRecordedSeconds();
        existing->bIsGhostPartial = Recorder->IsPartialGhost();
      }
    }

    const auto SavedResult = std::find_if(
        gi->multiplayer_results.begin(),
        gi->multiplayer_results.end(),
        [ConnectionId](const GI_main::FMultiplayerResult& Result) {
          return Result.ConnectionId == ConnectionId;
        }
    );
    if (SavedResult != gi->multiplayer_results.end()) {
      M_LOG(
          Log,
          "Race result collected: connection={}, identity={}, score={}, ghost_bytes={}, "
          "recorded_seconds={}, partial={}",
          ConnectionId,
          SavedResult->IdentityKey,
          SavedResult->FinishTime,
          SavedResult->GhostData.size(),
          SavedResult->GhostRecordedSeconds,
          SavedResult->bIsGhostPartial
      );
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
  for (const auto& [ConnectionId, Recorder] : GhostRecorders) {
    (void)ConnectionId;
    if (Recorder) {
      Recorder->StopRecording();
    }
  }
  GetWorld()->ServerTravel(GameSceneIds::Clear);
}

void AGameSceneBase::RaceStart() {
  M_LOG(Log, "start", 0);

  RaceRunning = true;
  for (const auto& [ConnectionId, Recorder] : GhostRecorders) {
    (void)ConnectionId;
    if (Recorder) {
      Recorder->StartRecording();
    }
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
