#pragma once
#include "Actor.h"
#include <vector> 
#include <string>

class ACSVMap : public AActor
{
public:
    ACSVMap(const std::string& csvPath, const std::string& chipPath, int xNum, int yNum, int xSize, int ySize);
    ACSVMap();

private:
    std::vector<int> m_ChipHandles;
    bool LoadCSV(const std::string& path, int xSize, int ySize, int totalChips);
};