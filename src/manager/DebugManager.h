#pragma once
#include <string>
#include <map>
#include<filesystem>
#include <sstream>
#include <format>
#include <type_traits>
#include <utility>
#define DebugMng DebugManager::GetInstance()
#define DSetLog(...) DebugManager::GetInstance().SetLogWithLoc(__FILE__, __LINE__, __VA_ARGS__)
class DebugManager {
private:
    DebugManager() = default;
    ~DebugManager() = default;

    struct DebugLogEntry {
        std::string value;
        float remainingSeconds;
    };

    std::map<std::string, DebugLogEntry> debugLogs;
    int lastDrawTimeMs = -1;

public:
    static DebugManager& GetInstance() {
        static DebugManager instance;
        return instance;
    }

    void SetLog(const std::string& key, const std::string& value, float durationSeconds = 0.0f);
    void SetLog(const std::string& key, int value);
    void SetLog(const std::string& key, float value);

    template<typename T>
    void SetLog(const std::string& key, const T& value, float durationSeconds = 0.0f) {
        std::ostringstream oss;
        oss << value;
        SetLog(key, oss.str(), durationSeconds);
    }

    template<typename T>
    void SetLogWithLoc(const std::string& filePath, int line, const T& value) {

        // パスからファイル名のみを抽出
        std::string fileName = std::filesystem::path(filePath).filename().string();

        // "ファイル名(行数)" の形式で登録
        std::string location = fileName + "("+ std::to_string(line) +")";
        SetLog(location, value);
    }

    template<typename T>
    void SetLogWithLoc(const std::string& filePath, int line, float durationSeconds, const T& value) {

        std::string fileName = std::filesystem::path(filePath).filename().string();
        std::string location = fileName + "("+ std::to_string(line) +")";
        SetLog(location, value, durationSeconds);
    }

    template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 0)>>
    void SetLogWithLoc(const std::string& filePath, int line, const std::string& formatText, Args&&... args) {

        // パスからファイル名のみを抽出
        std::string fileName = std::filesystem::path(filePath).filename().string();

        // "ファイル名(行数)" の形式で登録
        std::string location = fileName + "("+ std::to_string(line) +")";
        SetLog(location, std::vformat(formatText, std::make_format_args(std::forward<Args>(args)...)));
    }

    template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) > 0)>>
    void SetLogWithLoc(const std::string& filePath, int line, float durationSeconds, const std::string& formatText, Args&&... args) {

        std::string fileName = std::filesystem::path(filePath).filename().string();
        std::string location = fileName + "("+ std::to_string(line) +")";
        SetLog(location, std::vformat(formatText, std::make_format_args(std::forward<Args>(args)...)), durationSeconds);
    }
    // 蓄積されたログをすべて描画する
    void Draw();

    // ログをリセットする（必要に応じて呼び出し）
    void Clear();
};
