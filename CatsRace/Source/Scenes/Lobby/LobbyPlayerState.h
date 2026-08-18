#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>

#include "Actor.h"
#include "Core/PlayerColorPalette.h"

class ALobbyPlayerState : public AActor {
 public:
  DEFINE_ACTOR_CLASS(ALobbyPlayerState)

  using FCallbackHandle = size_t;
  using FSelectedMapChangedCallback = std::function<void(const std::string&)>;

  ALobbyPlayerState();
  ALobbyPlayerState(const FVector2D& Location, FRotator Rotation);

  bool SetPlayerName(const std::string& Name);
  bool SetDeviceId(const std::string& InDeviceId);
  void SetPlayerColorIndex(uint8_t InColorIndex);
  void SetLobbyOptions(const std::string& InSelectedLevelPath, int InMaxPlayers);
  void SetFinishResult(bool bInFinished, float InFinishTime);
  void SetStartCountdownSeconds(int InStartCountdownSeconds);
  void RequestRaceGhostDownload(const std::string& MapId, int MapVersion);

  FCallbackHandle AddOnSelectedMapChanged(FSelectedMapChangedCallback Callback);
  void RemoveOnSelectedMapChanged(FCallbackHandle Handle);

  const std::string& GetPlayerName() const { return PlayerName; }
  const std::string& GetDeviceId() const { return DeviceId; }
  uint8_t GetPlayerColorIndex() const { return PlayerColorIndex; }
  const std::string& GetSelectedLevelPath() const { return SelectedLevelPath; }
  int GetMaxPlayers() const { return MaxPlayers; }
  bool IsFinished() const { return bFinished; }
  float GetFinishTime() const { return FinishTime; }
  int GetStartCountdownSeconds() const { return StartCountdownSeconds; }

 private:
  void InitializeRPCs();
  void ApplyPlayerName(const std::string& Name);
  void ApplyDeviceId(const std::string& InDeviceId);
  void ApplyLobbyOptions(std::string InSelectedLevelPath, int InMaxPlayers);
  void ApplyFinishResult(bool bInFinished, float InFinishTime);
  void SetSelectedMap(const std::string& NewLevelPath);
  void OnRepSelectedLevelPath(std::string OldLevelPath);
  void BroadcastOnSelectedMapChanged();
  void ClientDownloadRaceGhosts(std::string MapId, int MapVersion);
  void ServerAcknowledgeRaceGhosts();
  void CompleteRaceGhostDownload();

  std::string PlayerName = "Player";
  std::string DeviceId;
  uint8_t PlayerColorIndex = InvalidPlayerColorIndex;
  std::string SelectedLevelPath;
  int MaxPlayers = 4;
  bool bFinished = false;
  float FinishTime = 0.0f;
  int StartCountdownSeconds = -1;
  FCallbackHandle NextSelectedMapChangedHandle = 1;
  std::unordered_map<FCallbackHandle, FSelectedMapChangedCallback> SelectedMapChangedCallbacks;
};
