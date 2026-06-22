#pragma once
#include "GameModeBase.h"
#include <TimerHandle.h>
#include <TimerManager.h>
#include <string>

class WCountDown;
class WMainHUD;
class WPauseMenu;
class MGhostRecorderComponent;
class AGhostPlayer;

class AGameSceneBase : public AGameModeBase
{
public:
	AGameSceneBase(std::string mapId, std::string levelFileName, FVector2D playerStartLocation);
	void OnUpdate(float DeltaTime) override;
	void BeginPlay() override;
	float GetRaceTime() const { return RaceTime; }
	const std::string& GetMapId() const { return MapId; }

	void RaceFinish();
	virtual void TogglePause();
	virtual void RestartGame();
	virtual void ReturnToTitle();
protected:
	virtual void OpenCurrentScene() = 0;

	void RaceCountDown();
	void RaceStart();
	void ClearCountDown();
	void LoadTopGhost();

	int m_CountDown = 3;
	FTimerHandle CountHandle;

	WCountDown* m_CountDownWidget = nullptr;
	WMainHUD* m_MainHUD = nullptr;
	WPauseMenu* m_PauseMenu = nullptr;
	MGhostRecorderComponent* m_GhostRecorder = nullptr;
	AGhostPlayer* m_GhostPlayer = nullptr;

	float RaceTime = 0.0f;
	bool RaceRunning = false;
	bool bPaused = false;
	std::string MapId;
	std::string LevelFileName;
	FVector2D PlayerStartLocation;
};
