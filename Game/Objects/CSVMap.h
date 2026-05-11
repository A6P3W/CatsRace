#pragma once
#include "Actor.h"
#include <vector> 
#include <string>

class ACSVMap : public AActor
{
public:
    ACSVMap(FVector2D location, FRotator rotation);
    ACSVMap();

private:
    std::vector<int> m_ChipHandles;
    bool LoadCSV(const std::string& path, int xSize, int ySize, int totalChips);
};