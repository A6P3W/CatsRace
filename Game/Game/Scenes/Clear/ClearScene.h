#pragma once
#include "GameModeBase.h"
#include <vector>

class WClearHUD;
class WNameSelectDialog;
class WNameInputDialog;
class WOverwriteConfirmDialog;
class WPostGameDialog;
class ALobbyPlayerState;

class AClearScene : public AGameModeBase
{
public:
	AClearScene();
	void OnUpdate(float DeltaTime) override;
	void Draw() override;

protected:
	void BeginPlay() override;

private:
	void ShowNameFlow();
	void ShowNameInputDialog();
	void CheckDuplicateAndPost(const std::string& name);
	void ExecutePostScore(const std::string& name);
	void FetchAndDisplay();
	void ShowPostGameDialog();
	void SpawnResultStatesFromGameInstance();
	std::vector<ALobbyPlayerState*> GetResultStates();

	WClearHUD* m_ClearHUD = nullptr;
	std::vector<std::string> m_FetchedUserIds;
	WNameSelectDialog* m_NameSelectDialog = nullptr;
	WNameInputDialog* m_NameInputDialog = nullptr;
	WOverwriteConfirmDialog* m_OverwriteDialog = nullptr;
	WPostGameDialog* m_PostGameDialog = nullptr;
};

