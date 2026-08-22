#include "Core/GI_main.h"

#include "Core/CatsRacePacketType.h"
#include "Log.h"
#include "NetBuffer.h"
#include "NetPacketType.h"
#include "NetworkManager.h"

GI_main::GI_main() {
  NetworkPacketCallbackHandle = NetworkManager::GetInstance().AddOnPacketReceived(
      [this](FNetworkConnectionId ConnectionId, FNetBuffer& Buffer) {
        HandleNetworkPacket(ConnectionId, Buffer);
      }
  );
}

GI_main::~GI_main() {
  if (NetworkPacketCallbackHandle != 0) {
    NetworkManager::GetInstance().RemoveOnPacketReceived(NetworkPacketCallbackHandle);
  }
}

void GI_main::HandleNetworkPacket(FNetworkConnectionId ConnectionId, FNetBuffer& Buffer) {
  (void)ConnectionId;
  ENetPacketType PacketType = ENetPacketType::None;
  if (!Buffer.Read(PacketType) || PacketType != ENetPacketType::UserMessage) {
    return;
  }
  uint8_t MessageType = 0;
  if (!Buffer.Read(MessageType)) {
    return;
  }
  if (MessageType == static_cast<uint8_t>(ECatsRaceMessageType::RaceStartTime)) {
    double StartTime = 0.0;
    if (Buffer.Read(StartTime)) {
      SetPendingRaceStartTime(StartTime);
    }
  }
}

bool GI_main::ConsumePendingRaceStartTime(double& OutStartTime) {
  if (!bHasPendingRaceStartTime) {
    return false;
  }
  OutStartTime = PendingRaceStartTime;
  bHasPendingRaceStartTime = false;
  return true;
}

void GI_main::SetPendingRaceStartTime(double StartTime) {
  PendingRaceStartTime = StartTime;
  bHasPendingRaceStartTime = true;
  M_LOG(Log, "[GI_main] Pending race start time stored: {}", StartTime);
}

void GI_main::ClearPendingRaceStartTime() {
  PendingRaceStartTime = 0.0;
  bHasPendingRaceStartTime = false;
}