#pragma once
#include <map>
#include <string>

class ResourceManager {
public:
    // シングルトンインスタンスの取得
    static ResourceManager& GetInstance() {
        static ResourceManager instance;
        return instance;
    }

    bool Init();

    // 画像ハンドルの取得
    int GetGraph(const std::string& path);

    // 全リソースの解放
    void ReleaseAll();

private:
    ResourceManager() = default;
    ~ResourceManager();

    // パスをキー、画像ハンドルを値として保持
    std::map<std::string, int> graphMap;
    int ErrorHandle;
};