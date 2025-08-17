#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetRegistry/AssetData.h"
#include "TextureFootprint.h"
#include "TextureBudgetScanner.generated.h"

/* Delegate C++ puro (não precisa de reflexão) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTextureScanFinished,
    const TArray<FTextureFootprint>& /*Results*/);

UCLASS()
class SMARTTEXTUREBUDGETERPLUGIN_API UTextureBudgetScanner : public UObject
{
    GENERATED_BODY()

public:
    /* Escaneia todas as UTexture2D do projeto (sincrono) */
    UFUNCTION(BlueprintCallable, Category = "TextureBudget")
    void ScanProject();

    /* Quem quiser ouvir o término do scan */
    FOnTextureScanFinished OnScanFinished;

private:
    void HandleAsset(const FAssetData& Asset, TArray<FTextureFootprint>& OutArray) const;
};
