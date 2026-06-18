#pragma once
#include "GameModeBase.h"

class WClearHUD;
class WNameSelectDialog;
class WNameInputDialog;
class WOverwriteConfirmDialog;

class AClearScene : public AGameModeBase
{
public:
	AClearScene();
	void OnUpdate(float DeltaTime) override;

protected:
	void BeginPlay() override;

private:
	void ShowNameFlow();
	void ShowNameInputDialog();
	void CheckDuplicateAndPost(const std::string& name);
	void ExecutePostScore(const std::string& name);
	void FetchAndDisplay();

	WClearHUD* m_ClearHUD = nullptr;
	std::vector<std::string> m_FetchedUserIds;
	WNameSelectDialog* m_NameSelectDialog = nullptr;
	WNameInputDialog* m_NameInputDialog = nullptr;
	WOverwriteConfirmDialog* m_OverwriteDialog = nullptr;
};

