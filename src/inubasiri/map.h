#pragma once
#include <vector>
#include <string>

class Map {
public:
    static constexpr int TILE_SIZE = 64; // 1タイルのサイズ（ピクセル）

    Map();
    ~Map();

    bool Load(const std::string& csvPath);
    void Draw();

private:
    std::vector<std::vector<int>> mapData;
    int chipGraphHandle; // マップチップ画像（複数のタイルが並んだ画像）
};