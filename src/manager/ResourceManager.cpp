#include <DxLib.h>
#include "ResourceManager.h"

bool ResourceManager::Init() {
    ErrorHandle = LoadGraph(".default/file-exclamation-point.png");
    return ErrorHandle != -1;
}

int ResourceManager::GetGraph(const std::string& path) {
    // すでに読み込み済みか検索
    auto it = graphMap.find(path);
    if (it != graphMap.end()) {
        return it->second; // キャッシュされているハンドルを返す
    }

    // 未読み込みならロードする
    int handle = LoadGraph(path.c_str());
    if (handle == -1) {
        return ErrorHandle;
    }
    graphMap[path] = handle;
    return handle;
}

void ResourceManager::ReleaseAll() {
    for (auto& pair : graphMap) {
        DeleteGraph(pair.second);
    }
    graphMap.clear();
}
ResourceManager::~ResourceManager() {
    ReleaseAll();
}