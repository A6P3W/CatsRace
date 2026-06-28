#include "GameSceneBase.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "ObjectManager.h"
#include "Scenes/Game/Player.h"
#include "Objects/SampleA.h"
#include "UMath.h"
#include "Log.h"
#include <PlayerController.h>
#include "PC_Game.h"
#include "ResourceManager.h"
#include <TimerHandle.h>
#include <TimerManager.h>
#include <SoundManager.h>
#include <PlayerStart.h>
#include <SceneManager.h>
#include "Scenes/Clear/ClearScene.h"
#include <UIManager.h>
#include "Scenes/Game/UI/WCountDown.h"
#include "Scenes/Game/UI/WMainHUD.h"
#include "Scenes/Game/UI/WPauseMenu.h"
#include "Scenes/Title/TitleScene.h"
#include <EnhancedInputComponent.h>
#include "Core/GI_main.h"
#include "Core/GameSceneIds.h"
#include "Ghost/GhostRecorderComponent.h"
#include "Ghost/GhostData.h"
#include "Ghost/GhostPlaybackComponent.h"
#include "Ghost/GhostPlayer.h"
#include "Services/LeaderBoardManager.h"
#include <NetworkManager.h>

AGameSceneBase::AGameSceneBase(std::string mapId, std::string levelFileName, FVector2D playerStartLocation)
	: MapId(std::move(mapId))
	, LevelFileName(std::move(levelFileName))
	, PlayerStartLocation(playerStartLocation)
{
	DefaultPawnClass = APlayer::StaticClassName();
	DefaultPlayerControllerClass = PC_Game::StaticClassName();
}

void AGameSceneBase::OnUpdate(float DeltaTime)
{
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

void AGameSceneBase::BeginPlay()
{
	AGameModeBase::BeginPlay();


	if (GetWorld()->IsServer()) {
		SpawnNetworkPlayer(0);
	}

	SpawnActor<ASampleA>();
	LoadTopGhost();
	M_LOG("Game scene initialized: {}", MapId);

	if (GetWorld()->IsServer()) {
		GetWorldTimerManager().SetTimer(CountHandle, this, &AGameSceneBase::RaceCountDown, 1.0f, true, 1.0f);
	}
}

void AGameSceneBase::LoadTopGhost()
{
	auto* lbm = GetWorld()->SpawnActor<LeaderBoardManager>();
	const std::string mapId = MapId;
	lbm->FetchLeaderBoard(mapId, [this, lbm, mapId](bool bSuccess, const std::vector<FLeaderBoardEntry>& entries) {
		if (!bSuccess || entries.empty()) {
			M_LOG("Top ghost skipped: leaderboard is empty or unavailable");
			return;
		}

		const std::string topUserId = entries.front().user_id;
		lbm->FetchGhostData(mapId, { topUserId }, [this, topUserId](bool bGhostSuccess, const std::unordered_map<std::string, std::string>& ghostDataById) {
			auto ghostDataIt = ghostDataById.find(topUserId);
			if (!bGhostSuccess || ghostDataIt == ghostDataById.end() || ghostDataIt->second.empty()) {
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
		});
	});
}

void AGameSceneBase::RaceFinish()
{
	if (auto* player = dynamic_cast<APlayer*>(GetPlayerPawn())) {
		NotifyPlayerFinished(player);
	}
}

APlayerController* AGameSceneBase::OnClientConnected(FNetworkConnectionId ConnectionId)
{
	return SpawnNetworkPlayer(ConnectionId);
}

void AGameSceneBase::OnClientDisconnected(FNetworkConnectionId ConnectionId)
{
	if (!GetWorld() || !GetWorld()->GetObjectManager()) {
		return;
	}
	for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
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

APlayerController* AGameSceneBase::SpawnNetworkPlayer(FNetworkConnectionId ConnectionId)
{
	if (!FindPlayerStart(ConnectionId)) {
		const float offset = static_cast<float>(ConnectionId) * 90.0f;
		SpawnActor<APlayerStart>({ PlayerStartLocation.X + offset, PlayerStartLocation.Y });
	}

	APlayerController* controller = SpawnDefaultPlayer(ConnectionId);
	APlayer* player = controller ? dynamic_cast<APlayer*>(controller->GetPawn()) : nullptr;
	if (!player) {
		return controller;
	}

	player->SetCanMove(RaceRunning);

	if (ConnectionId == 0) {
		auto recorder = std::make_unique<MGhostRecorderComponent>();
		m_GhostRecorder = recorder.get();
		player->AddComponent(std::move(recorder));
	}

	if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
		auto existing = std::find_if(gi->multiplayer_results.begin(), gi->multiplayer_results.end(), [ConnectionId](const GI_main::FMultiplayerResult& result) {
			return result.ConnectionId == ConnectionId;
		});
		if (existing == gi->multiplayer_results.end()) {
			GI_main::FMultiplayerResult result;
			result.ConnectionId = ConnectionId;
			result.PlayerName = ConnectionId == 0 && !gi->player_name.empty() ? gi->player_name : ("Player " + std::to_string(ConnectionId + 1));
			gi->multiplayer_results.push_back(result);
		}
	}

	return controller;
}

void AGameSceneBase::NotifyPlayerFinished(APlayer* Player)
{
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

void AGameSceneBase::SaveResult(FNetworkConnectionId ConnectionId, float FinishTime)
{
	if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
		gi->ClearTime = FinishTime;
		gi->map_id = MapId;
		gi->LastGhostData = (ConnectionId == 0 && m_GhostRecorder) ? m_GhostRecorder->GetSerializedData() : "";
		auto existing = std::find_if(gi->multiplayer_results.begin(), gi->multiplayer_results.end(), [ConnectionId](const GI_main::FMultiplayerResult& result) {
			return result.ConnectionId == ConnectionId;
		});
		if (existing == gi->multiplayer_results.end()) {
			GI_main::FMultiplayerResult result;
			result.ConnectionId = ConnectionId;
			result.PlayerName = ConnectionId == 0 && !gi->player_name.empty() ? gi->player_name : ("Player " + std::to_string(ConnectionId + 1));
			result.bFinished = true;
			result.FinishTime = FinishTime;
			gi->multiplayer_results.push_back(result);
		}
		else if (!existing->bFinished) {
			existing->bFinished = true;
			existing->FinishTime = FinishTime;
		}
	}
}

bool AGameSceneBase::AreAllPlayersFinished() const
{
	if (auto* gi = dynamic_cast<GI_main*>(SceneManager::GetInstance().GetGameInstance())) {
		if (gi->multiplayer_results.empty()) {
			return false;
		}
		return std::all_of(gi->multiplayer_results.begin(), gi->multiplayer_results.end(), [](const GI_main::FMultiplayerResult& result) {
			return result.bFinished;
		});
	}
	return false;
}

void AGameSceneBase::TravelToClear()
{
	if (bResultTravelRequested || !GetWorld()->IsServer()) {
		return;
	}
	bResultTravelRequested = true;
	RaceRunning = false;
	GetWorld()->ServerTravel(GameSceneIds::Clear);
}

void AGameSceneBase::RaceCountDown()
{
	m_CountDown--;

	if (m_CountDown <= 0) {
		GetWorldTimerManager().ClearTimer(CountHandle);
		RaceStart();
	}
}

void AGameSceneBase::RaceStart()
{
	M_LOG("start", 0);

	RaceRunning = true;
	if (m_GhostRecorder) {
		m_GhostRecorder->StartRecording();
	}

	if (GetWorld() && GetWorld()->GetObjectManager()) {
		for (const auto& actorPtr : GetWorld()->GetObjectManager()->GetAllActors()) {
			if (auto* player = dynamic_cast<APlayer*>(actorPtr.get())) {
				player->SetCanMove(true);
			}
		}
	}
}

void AGameSceneBase::RestartGame()
{
	OpenCurrentScene();
}

void AGameSceneBase::ReturnToTitle()
{
	SceneManager::GetInstance().OpenSceneById(GameSceneIds::Menu);
}

