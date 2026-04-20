#include <DxLib.h>
#include "DebugManager.h"

void DebugManager::SetLog(const std::string& key, const std::string& value) {
    debugLogs[key] = value;
}

void DebugManager::SetLog(const std::string& key, int value) {
    debugLogs[key] = std::to_string(value);
}

void DebugManager::SetLog(const std::string& key, float value) {
    debugLogs[key] = std::to_string(value);
}

void DebugManager::Draw() {
	clsDx();
    // 描画位置の初期化（左上など）
    // printfDxを使う場合は clsDx() との組み合わせに注意してください

    for (const auto& [key, value] : debugLogs) {
        printfDx("%s : %s\n", key.c_str(), value.c_str());
    }
    debugLogs.clear();
}

void DebugManager::Clear() {
    debugLogs.clear();
}