#pragma once
#if WITH_EDITOR

#include "Widgets/SCompoundWidget.h"
#include "AssetThumbnail.h"
#include "TextureFootprint.h"

class UTextureBudgetScanner;

class STextureBudgetWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STextureBudgetWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    /* UI --------------------------------------------------- */
    FReply OnScanClicked();
    void   HandleFootprintReady(const FTextureFootprint& FP);
    void   HandleScanFinished();
    void   OpenAsset(const TSharedPtr<FTextureFootprint>& Item);

    /* Estado ----------------------------------------------- */
    bool bIsScanning = false;
    double LastRefresh = 0.0;

    /* Widgets ---------------------------------------------- */
    TSharedPtr<SListView<TSharedPtr<FTextureFootprint>>> ListView;
    TSharedPtr<class SThrobber> Spinner;

    /* Dados ------------------------------------------------ */
    TArray<TSharedPtr<FTextureFootprint>> RowData;

    /* Scanner UObject -------------------------------------- */
    TObjectPtr<UTextureBudgetScanner> Scanner;

    /* Thumbnails ------------------------------------------- */
    static TSharedPtr<FAssetThumbnailPool> ThumbPool;
};

#endif /* WITH_EDITOR */
