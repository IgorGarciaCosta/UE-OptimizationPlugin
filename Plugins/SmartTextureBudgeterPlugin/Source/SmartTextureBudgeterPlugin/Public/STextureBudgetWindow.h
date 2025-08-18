#pragma once
#if WITH_EDITOR

#include "Widgets/SCompoundWidget.h"
#include "AssetThumbnail.h"
#include "TextureFootprint.h"

class UTextureBudgetScanner;

/* modos de ordenação -------------------------------------------------- */
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

    /* busca ------------------------------------------------------------- */
    void   OnSearchChanged(const FText& NewText);
    void   RebuildFilteredList();

    /* ordenação --------------------------------------------------------- */
    TSharedRef<SWidget> BuildSortMenu();
    void   OnSortChosen(ESortMode InMode);
    void   ApplySort();
    FText  GetSortLabel() const;

    /* item -------------------------------------------------------------- */
    void   OpenAsset(const TSharedPtr<FTextureFootprint>& Item);
    void   OpenResizeDialog(const TSharedPtr<FTextureFootprint>& Item);

    /* Estado ------------------------------------------------------------ */
    bool      bIsScanning = false;
    double    LastRefresh = 0.0;
    ESortMode CurrentSort = ESortMode::Ascending;
    FString   CurrentFilter;

    /* Widgets ----------------------------------------------------------- */
    TSharedPtr<SListView<TSharedPtr<FTextureFootprint>>> ListView;
    TSharedPtr<class SThrobber>  Spinner;
    TSharedPtr<class SSearchBox> SearchBox;

    /* Dados ------------------------------------------------------------- */
    TArray<TSharedPtr<FTextureFootprint>> AllRows;
    TArray<TSharedPtr<FTextureFootprint>> RowData;

    /* Scanner ----------------------------------------------------------- */
    TObjectPtr<UTextureBudgetScanner> Scanner;

    /* Thumbnails -------------------------------------------------------- */
    static TSharedPtr<FAssetThumbnailPool> ThumbPool;

    TWeakPtr<class SWindow> ActiveResizeWindow;
};

#endif /* WITH_EDITOR */
