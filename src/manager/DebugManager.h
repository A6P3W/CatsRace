#pragma once
#include <string>
#include <map>

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

    // 蓄積されたログをすべて描画する
    void Draw();

    // ログをリセットする（必要に応じて呼び出し）
    void Clear();
};