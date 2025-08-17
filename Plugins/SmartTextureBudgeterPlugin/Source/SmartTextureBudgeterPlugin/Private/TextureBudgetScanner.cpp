#include "TextureBudgetScanner.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"

/* ----------------------------------------------------------------- */
void UTextureBudgetScanner::StartAsyncScan(int32 InBatchSize)
{
    if (bScanning) return;

    BatchSize = FMath::Max(1, InBatchSize);
    CurrentIndex = 0;
    Assets.Empty();

    /* 1. Pega todos os AssetData de UTexture2D (rápido, sem carga) */
    FAssetRegistryModule& ARM =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    FARFilter Filter;
    Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
    Filter.bRecursiveClasses = true;

    ARM.Get().GetAssets(Filter, Assets);

    bScanning = true;
    LoadNextBatch();
}

/* ----------------------------------------------------------------- */
void UTextureBudgetScanner::CancelScan()
{
    if (!bScanning) return;

    if (CurrentHandle.IsValid())
    {
        CurrentHandle->CancelHandle();
        CurrentHandle.Reset();
    }
    Assets.Empty();
    bScanning = false;
}

/* ----------------------------------------------------------------- */
void UTextureBudgetScanner::LoadNextBatch()
{
    if (CurrentIndex >= Assets.Num())
    {
        /* terminou */
        bScanning = false;
        Assets.Empty();
        CurrentHandle.Reset();
        OnFinished.Broadcast();
        return;
    }

    /* Paths do próximo lote ------------------------------------- */
    CurrentPaths.Empty();
    for (int32 i = 0; i < BatchSize && CurrentIndex < Assets.Num(); ++i, ++CurrentIndex)
    {
        CurrentPaths.Add(Assets[CurrentIndex].ToSoftObjectPath());
    }

    CurrentHandle = Streamable.RequestAsyncLoad(
        CurrentPaths,
        FStreamableDelegate::CreateUObject(this, &UTextureBudgetScanner::HandleBatchLoaded),
        FStreamableManager::AsyncLoadHighPriority);
}

/* ----------------------------------------------------------------- */
void UTextureBudgetScanner::HandleBatchLoaded()
{
    for (const FSoftObjectPath& Path : CurrentPaths)
    {
        UTexture2D* Tex = Cast<UTexture2D>(Path.ResolveObject());
        if (!Tex) continue;

        Tex->FinishCachePlatformData();   // rápido; compilação vai em thread própria

        FTextureFootprint FP;
        FP.AssetData = FAssetData(Tex);
        FP.Width = Tex->GetSizeX();
        FP.Height = Tex->GetSizeY();
        FP.ResourceSizeBytes = Tex->GetResourceSizeBytes(
            EResourceSizeMode::EstimatedTotal);

        OnFootprintReady.Broadcast(FP);
    }

    /* Próximo lote ------------------------------------------------ */
    CurrentHandle.Reset();
    LoadNextBatch();
}
