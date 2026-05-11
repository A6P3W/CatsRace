#include "CSVMap.h"
#include "SpriteComponent.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <DxLib.h>
#include <RenderSystem.h>

ACSVMap::ACSVMap(FVector2D location, FRotator rotation)
{
    SetActorLocation(location);
    int totalChips =64;
    SetActorScale(5);
    m_ChipHandles.resize(totalChips);
    LoadDivGraph("images/map.bmp", totalChips, 8, 8, 32, 32, m_ChipHandles.data(), TRUE);

    LoadCSV("images/otamesi.csv", 32, 32, totalChips);
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
                    tile->SetRelativeLocation({ GetActorLocation().X+(float)x * (float)0.97 * xSize, GetActorLocation().Y+(float)y * (float)0.97 * ySize });
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
