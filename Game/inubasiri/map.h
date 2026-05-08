#pragma once
#include <vector>
#include <string>

class Map {
public:
    static constexpr int TILE_SIZE = 32; // 1タイルのサイズ（ピクセル）

    Map();
    ~Map();

   
    bool Load(const std::string& csvPath, const std::string& chipPath);
    void Draw(int offsetX, int offsetY);

private:
    std::vector<std::vector<int>> mapData;
    int chipGraphHandle; // マップチップ画像ハンドル
    int mapWidth;        // マップの横方向タイル数
    int mapHeight;       // マップの縦方向タイル数
private:
    static constexpr int TILE_SIZE = 32; // タイルサイズ（適宜調整）
    std::vector<std::vector<int>> mapData;
    int chipGraphHandle;
    int mapWidth, mapHeight;
};