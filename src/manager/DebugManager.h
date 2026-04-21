#pragma once
#include <string>
#include <map>
#include<filesystem>
#define DebugMng DebugManager::GetInstance()
#define DSetLog(key, value) DebugManager::GetInstance().SetLogWithLoc(__FILE__, __LINE__, key, value)
class DebugManager {
private:
    DebugManager() = default;
    ~DebugManager() = default;

    // キーと値を保持するマップ
    std::map<std::string, std::string> debugLogs;

public:
    static DebugManager& GetInstance() {
        static DebugManager instance;
        return instance;
    }

    // 値をセットする関数（文字列版と数値版があると便利です）
    void SetLog(const std::string& key, const std::string& value);
    void SetLog(const std::string& key, int value);
    void SetLog(const std::string& key, float value);
    void SetLogWithLoc(const std::string& filePath, int line, const std::string& key, const std::string& value) {
        // パスからファイル名のみを抽出
        std::string fileName = std::filesystem::path(filePath).filename().string();

        // "ファイル名(行数) キー" の形式で登録
        std::string location = fileName + "("+ std::to_string(line) +") " + key;
        SetLog(location, value);
    }
    // 蓄積されたログをすべて描画する
    void Draw();

    // ログをリセットする（必要に応じて呼び出し）
    void Clear();
};
