#pragma once
#include <vector>
#include <string>

class Map {
public:
    static constexpr int TILE_SIZE = 32; // 1タイルのサイズ（ピクセル）

    Map();
    ~Map();

   
    bool Load(const std::string& csvPath, const std::string& chipPath);
    void Draw();

private:
    std::vector<std::vector<int>> mapData;
    int chipGraphHandle; // マップチップ画像ハンドル
    int mapWidth;        // マップの横方向タイル数
    int mapHeight;       // マップの縦方向タイル数
};