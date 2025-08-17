#pragma once
#include "AssetRegistry/AssetData.h"

/* Dados mostrados na UI */
struct FTextureFootprint
{
    FAssetData AssetData;

    int32  Width = 0;   // Max-In-Game
    int32  Height = 0;
    int64  ResourceSizeBytes = 0;   // Memória estimada
};
