#pragma once
#if WITH_EDITOR

#include "Widgets/SCompoundWidget.h"
#include "AssetThumbnail.h"
#include "TextureFootprint.h"

class UTextureBudgetScanner;

/* modos de ordenação */
enum class ESortMode : uint8
{
    Ascending,
    Descending,
    Alphabetical
};

class STextureBudgetWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STextureBudgetWindow) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~STextureBudgetWindow() override;

private:
    /* UI callbacks ------------------------------------------------------ */
    FReply OnScanClicked();
    void   HandleFootprintReady(const FTextureFootprint& FP);
    void   HandleScanFinished();

    /* ordenação --------------------------------------------------------- */
    TSharedRef<SWidget> BuildSortMenu();
    void   OnSortChosen(ESortMode InMode);
    void   ApplySort();
    FText  GetSortLabel() const;                 // texto dinâmico do botão
    void   OpenAsset(const TSharedPtr<FTextureFootprint>& Item);

    /* Estado ------------------------------------------------------------ */
    bool      bIsScanning = false;
    double    LastRefresh = 0.0;
    ESortMode CurrentSort = ESortMode::Ascending;

    /* Widgets ----------------------------------------------------------- */
    TSharedPtr<SListView<TSharedPtr<FTextureFootprint>>> ListView;
    TSharedPtr<class SThrobber> Spinner;

    /* Dados ------------------------------------------------------------- */
    TArray<TSharedPtr<FTextureFootprint>> RowData;

    /* Scanner ----------------------------------------------------------- */
    TObjectPtr<UTextureBudgetScanner> Scanner;

    /* Thumbnails -------------------------------------------------------- */
    static TSharedPtr<FAssetThumbnailPool> ThumbPool;
};

#endif /* WITH_EDITOR */
