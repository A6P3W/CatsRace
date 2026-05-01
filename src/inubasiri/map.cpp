#include "map.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <DxLib.h>
#include "manager/ResourceManager.h"

Map::Map() : chipGraphHandle(-1), mapWidth(0), mapHeight(0) {

}
Map::~Map() {

}

bool Map::Load(const std::string& csvPath, const std::string& chipPath) {
	// 画像の読み込み
	chipGraphHandle = ResourceManager::GetInstance().GetGraph(chipPath);
	if (chipGraphHandle == -1) {
		printfDx("Error: Failed to load image [%s]\n", chipPath.c_str());
		return false;
	}
	//csv読み込み
	std::ifstream file(csvPath);
	if (!file.is_open()) {
		printfDx("Error: Failed to open CSV [%s]\n", csvPath.c_str());
		return false;
	}
	mapData.clear();
	std::string line;
	// Map::Load 内
	while (std::getline(file, line)) {
		// 改行コード \r を除去 (Windows対策)
		if (!line.empty() && line.back() == '\r') line.pop_back();

		std::vector<int> row;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, ',')) {
			if (!cell.empty()) {
				try {
					row.push_back(std::stoi(cell));
				}
				catch (...) {
					// 数値変換失敗時のログ
					continue;
				}
			}
		}
		if (!row.empty()) mapData.push_back(row);
	
	}
	return true;
}
void Map::Draw() {
	if (chipGraphHandle == -1)return;

	int imgW, imgH;
	GetGraphSize(chipGraphHandle, &imgW, &imgH);
	int chipsPerRow = imgW / TILE_SIZE;

	for (int y = 0; y < (int)mapData.size(); ++y) {
		for (int x = 0; x < (int)mapData[y].size(); ++x) {
			int chipID = mapData[y][x];
			if (chipID < 0) continue;

		int srcX = (chipID % chipsPerRow) * TILE_SIZE;
		int srcY = (chipID / chipsPerRow) * TILE_SIZE;

		int drawX = x * TILE_SIZE;
		int drawY = y * TILE_SIZE;

		DrawRectGraph(drawX, drawY, srcX, srcY, TILE_SIZE, TILE_SIZE, chipGraphHandle, TRUE);
		}
	}
}