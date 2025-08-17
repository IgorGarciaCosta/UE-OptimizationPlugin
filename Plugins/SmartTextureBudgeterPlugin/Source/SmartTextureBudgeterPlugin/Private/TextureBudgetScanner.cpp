#include "TextureBudgetScanner.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"

void UTextureBudgetScanner::ScanProject()
{
    TArray<FTextureFootprint> Results;

    /* 1. AssetRegistry */
    FAssetRegistryModule& Arm = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& Registry = Arm.Get();

    /* 2. Filtro para UTexture2D */
    FARFilter Filter;
    Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;

    TArray<FAssetData> Assets;
    Registry.GetAssets(Filter, Assets);

    /* 3. Processa cada asset */
    Results.Reserve(Assets.Num());
    for (const FAssetData& AD : Assets)
    {
        HandleAsset(AD, Results);
    }

    UE_LOG(LogTemp, Log, TEXT("[STB] ScanProject encontrou %d texturas"), Results.Num());
    OnScanFinished.Broadcast(Results);
}

void UTextureBudgetScanner::HandleAsset(const FAssetData& Asset,
    TArray<FTextureFootprint>& OutArray) const
{
    FTextureFootprint FP;
    FP.AssetData = Asset;

    FString W, H;
    if (Asset.GetTagValue("ImportedSizeX", W) &&
        Asset.GetTagValue("ImportedSizeY", H))
    {
        FP.Width = FCString::Atoi(*W);
        FP.Height = FCString::Atoi(*H);
        FP.BytesCurrent = FP.Width * FP.Height * 4;   // aproximação
    }

    OutArray.Add(FP);
}
