#include "CSVMap.h"
#include "SpriteComponent.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <DxLib.h>

ACSVMap::ACSVMap(const std::string& csvPath, const std::string& chipPath, int xNum, int yNum, int xSize, int ySize)
{
    int totalChips = xNum * yNum;
    m_ChipHandles.resize(totalChips);

    LoadDivGraph(chipPath.c_str(), totalChips, xNum, yNum, xSize, ySize, m_ChipHandles.data(), TRUE);

    LoadCSV(csvPath, xSize, ySize, totalChips);
}


bool ACSVMap::LoadCSV(const std::string& path, int xSize, int ySize, int totalChips)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::string line;
    int y = 0;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::stringstream ss(line);
        std::string cell;
        int x = 0;
        while (std::getline(ss, cell, ',')) {
            if (!cell.empty()) {
                int chipID = std::stoi(cell);
                if (chipID >= 0 && chipID < totalChips) {
                    auto tile = std::make_unique<MSpriteComponent>(-500, RenderSpace::World);

                    tile->SubmitGraph(1.0, m_ChipHandles[chipID], 255);

                    tile->SetParentComponent(this->GetRootComponent());
                    tile->SetRelativeLocation({ (float)x * (float)0.97 * xSize, (float)y * (float)0.97 * ySize });
                    AddComponent(std::move(tile));
                }
            }
            x++;
        }
        y++;
    }
    return true;
}

// デフォルト設定
ACSVMap::ACSVMap() : ACSVMap("images/otamesi.csv", "images/map.bmp", 8, 8, 32, 32) {}