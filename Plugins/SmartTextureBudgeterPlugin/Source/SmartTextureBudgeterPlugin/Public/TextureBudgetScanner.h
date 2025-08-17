#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetRegistry/AssetData.h"
#include "TextureFootprint.h"
#include "Engine/StreamableManager.h"
#include "TextureBudgetScanner.generated.h"

/* 1) Emite um footprint à medida que cada textura é processada
 * 2) Emite um sinal quando o scan terminou                                */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFootprintReady,
    const FTextureFootprint& /*FP*/);
DECLARE_MULTICAST_DELEGATE(FOnAsyncScanFinished);

UCLASS()
class SMARTTEXTUREBUDGETERPLUGIN_API UTextureBudgetScanner : public UObject
{
    GENERATED_BODY()

public:
    /* Inicia o scan.  Chamará os delegates acima.                         */
    void StartAsyncScan(int32 InBatchSize = 32);

    /* Cancela caso ainda esteja rodando                                    */
    void CancelScan();

    /* Delegates de saída                                                   */
    FOnFootprintReady    OnFootprintReady;
    FOnAsyncScanFinished OnFinished;

private:
    /* Passo interno: carrega o próximo lote                                */
    void LoadNextBatch();

    /* Callback de término de lote                                          */
    void HandleBatchLoaded();

    /* Estado ------------------------------------------------------------- */
    TArray<FAssetData>              Assets;          // todas as texturas
    int32                           CurrentIndex = 0;
    int32                           BatchSize = 32;
    bool                            bScanning = false;

    /* Streaming ---------------------------------------------------------- */
    FStreamableManager              Streamable;
    TSharedPtr<FStreamableHandle>   CurrentHandle;
    TArray<FSoftObjectPath>         CurrentPaths;    // paths do lote
};
