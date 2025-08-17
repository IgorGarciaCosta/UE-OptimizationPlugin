#pragma once
#if WITH_EDITOR

#include "Widgets/SCompoundWidget.h"
#include "TextureFootprint.h"
#include "AssetThumbnail.h"

class UTextureBudgetScanner;

class STextureBudgetWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STextureBudgetWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    /* UI callbacks */
    FReply OnScanClicked();
    void   HandleScanFinished(const TArray<FTextureFootprint>& Results);

    /* ListView + dados */
    TSharedPtr<SListView<TSharedPtr<FTextureFootprint>>> ListView;
    TArray<TSharedPtr<FTextureFootprint>> RowData;

    /* Scanner UObject */
    TObjectPtr<UTextureBudgetScanner> Scanner;

    /* Pool de thumbnails compartilhado pelo widget */
    static TSharedPtr<FAssetThumbnailPool> ThumbPool;
};

#endif  // WITH_EDITOR
