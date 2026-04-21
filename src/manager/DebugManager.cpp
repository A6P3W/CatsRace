#include <DxLib.h>
#include "DebugManager.h"

void DebugManager::SetLog(const std::string& key, const std::string& value, float durationSeconds) {
    debugLogs[key] = { value, durationSeconds };
}

void DebugManager::SetLog(const std::string& key, int value) {
    SetLog(key, std::to_string(value));
}

void DebugManager::SetLog(const std::string& key, float value) {
    SetLog(key, std::to_string(value));
}

void DebugManager::Draw() {
	clsDx();

    const int nowMs = GetNowCount();
    const float deltaSeconds = (lastDrawTimeMs < 0) ? 0.0f : (nowMs - lastDrawTimeMs) / 1000.0f;
    lastDrawTimeMs = nowMs;

    for (auto it = debugLogs.begin(); it != debugLogs.end();) {
        printfDx("%s : %s\n", it->first.c_str(), it->second.value.c_str());

        if (it->second.remainingSeconds <= 0.0f) {
            it = debugLogs.erase(it);
            continue;
        }

        it->second.remainingSeconds -= deltaSeconds;
        if (it->second.remainingSeconds <= 0.0f) {
            it = debugLogs.erase(it);
        }
        else {
            ++it;
        }
    }
}

void DebugManager::Clear() {
    debugLogs.clear();
    lastDrawTimeMs = -1;
}