#pragma once
#include "AssetRegistry/AssetData.h"

/* Dados mínimos de cada textura encontrados no scanner */
struct FTextureFootprint
{
    FAssetData AssetData;      // mantém referência p/ gerar thumbnail e pegar nome

    int32 Width = 0;
    int32 Height = 0;
    int32 BytesCurrent = 0;
    int32 BytesAfter = 0;
};
