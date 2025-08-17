#include "STextureBudgetWindow.h"
#if WITH_EDITOR

#include "TextureBudgetScanner.h"
#include "AssetThumbnail.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SOverlay.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

TSharedPtr<FAssetThumbnailPool> STextureBudgetWindow::ThumbPool;

/*------------------------------------------------------------------------*/
void STextureBudgetWindow::Construct(const FArguments& InArgs)
{
    if (!ThumbPool.IsValid())
        ThumbPool = MakeShareable(new FAssetThumbnailPool(32));

    /* Scanner */
    Scanner = NewObject<UTextureBudgetScanner>();
    Scanner->AddToRoot();
    Scanner->OnFootprintReady.AddRaw(this, &STextureBudgetWindow::HandleFootprintReady);
    Scanner->OnFinished.AddRaw(this, &STextureBudgetWindow::HandleScanFinished);

    /* UI */
    ChildSlot
        [
            SNew(SOverlay)

                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                        /* --- Barra de botões ------------------------------------- */
                        + SVerticalBox::Slot().AutoHeight().Padding(4)
                        [
                            SNew(SHorizontalBox)

                                /* Scan */
                                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                                [
                                    SNew(SButton)
                                        .Text(FText::FromString(TEXT("Scan Project")))
                                        .OnClicked(this, &STextureBudgetWindow::OnScanClicked)
                                        .IsEnabled_Lambda([this]() { return !bIsScanning; })
                                ]

                                /* Sort combo */
                                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                                [
                                    SNew(SComboButton)
                                        .ButtonContent()
                                        [
                                            SNew(STextBlock)
                                                .Text_Lambda([this]() { return GetSortLabel(); })
                                        ]
                                        .OnGetMenuContent(this, &STextureBudgetWindow::BuildSortMenu)
                                ]

                                /* Search box */
                                + SHorizontalBox::Slot().FillWidth(1.f)
                                [
                                    SAssignNew(SearchBox, SSearchBox)
                                        .OnTextChanged(this, &STextureBudgetWindow::OnSearchChanged)
                                        .HintText(FText::FromString(TEXT("Search...")))
                                ]
                        ]

                        /* --- Lista ------------------------------------------------- */
                        + SVerticalBox::Slot().FillHeight(1.f).Padding(4)
                        [
                            SAssignNew(ListView, SListView<TSharedPtr<FTextureFootprint>>)
                                .ListItemsSource(&RowData)

                                .OnGenerateRow_Lambda([](TSharedPtr<FTextureFootprint> Item,
                                    const TSharedRef<STableViewBase>& Owner)
                                    {
                                        TSharedPtr<FAssetThumbnail> Thumb = MakeShareable(
                                            new FAssetThumbnail(Item->AssetData, 64, 64,
                                                STextureBudgetWindow::ThumbPool));

                                        const double SizeMB =
                                            static_cast<double>(Item->ResourceSizeBytes) / 1024.0 / 1024.0;

                                        const FString Label = FString::Printf(
                                            TEXT("%s   (%dx%d)   [%.2f MB]"),
                                            *Item->AssetData.AssetName.ToString(),
                                            Item->Width, Item->Height,
                                            SizeMB);

                                        return SNew(STableRow<TSharedPtr<FTextureFootprint>>, Owner)
                                            [
                                                SNew(SHorizontalBox)

                                                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                                    [
                                                        Thumb->MakeThumbnailWidget()
                                                    ]

                                                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0)
                                                    [
                                                        SNew(STextBlock).Text(FText::FromString(Label))
                                                    ]
                                            ];
                                    })

                                .OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FTextureFootprint> Item)
                                    {
                                        OpenAsset(Item);
                                    })
                        ]
                ]

                /* --- Spinner --------------------------------------------------- */
                + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SAssignNew(Spinner, SThrobber)
                        .Visibility_Lambda([this]()
                            {
                                return bIsScanning ? EVisibility::Visible : EVisibility::Collapsed;
                            })
                ]
        ];
}

/*------------------------------------------------------------------------*/
STextureBudgetWindow::~STextureBudgetWindow()
{
    if (Scanner)
    {
        Scanner->OnFootprintReady.RemoveAll(this);
        Scanner->OnFinished.RemoveAll(this);
        Scanner->CancelScan();
        Scanner->RemoveFromRoot();
        Scanner = nullptr;
    }
}

/*-------------------------- Scan ----------------------------------------*/
FReply STextureBudgetWindow::OnScanClicked()
{
    if (bIsScanning) return FReply::Handled();

    bIsScanning = true;
    AllRows.Empty();
    RowData.Empty();
    ListView->RequestListRefresh();
    LastRefresh = FPlatformTime::Seconds();

    Scanner->StartAsyncScan(32);
    return FReply::Handled();
}

void STextureBudgetWindow::HandleFootprintReady(const FTextureFootprint& FP)
{
    AllRows.Add(MakeShared<FTextureFootprint>(FP));

    const double Now = FPlatformTime::Seconds();
    if (Now - LastRefresh > 0.25)
    {
        RebuildFilteredList();
        LastRefresh = Now;
        ListView->RequestListRefresh();
    }
}

void STextureBudgetWindow::HandleScanFinished()
{
    RebuildFilteredList();
    ListView->RequestListRefresh();
    bIsScanning = false;
}

/*-------------------------- Busca ---------------------------------------*/
void STextureBudgetWindow::OnSearchChanged(const FText& NewText)
{
    CurrentFilter = NewText.ToString();
    RebuildFilteredList();
    ListView->RequestListRefresh();
}

void STextureBudgetWindow::RebuildFilteredList()
{
    /* aplica filtro + ordenação */
    RowData.Empty();

    const bool bHasFilter = !CurrentFilter.IsEmpty();
    const FString FilterLower = CurrentFilter.ToLower();

    for (const TSharedPtr<FTextureFootprint>& FP : AllRows)
    {
        if (!bHasFilter)
        {
            RowData.Add(FP);
        }
        else
        {
            const FString Name = FP->AssetData.AssetName.ToString().ToLower();
            if (Name.Contains(FilterLower))
                RowData.Add(FP);
        }
    }
    ApplySort();
}

/*-------------------------- Ordenação -----------------------------------*/
TSharedRef<SWidget> STextureBudgetWindow::BuildSortMenu()
{
    FMenuBuilder Menu(true, nullptr);

    auto AddEntry = [&](ESortMode Mode, const FString& Label)
        {
            Menu.AddMenuEntry(
                FText::FromString(Label),
                FText::GetEmpty(),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &STextureBudgetWindow::OnSortChosen, Mode))
            );
        };

    AddEntry(ESortMode::Ascending, TEXT("Ascending"));
    AddEntry(ESortMode::Descending, TEXT("Descending"));
    AddEntry(ESortMode::Alphabetical, TEXT("Alphabetical"));
    return Menu.MakeWidget();
}

void STextureBudgetWindow::OnSortChosen(ESortMode InMode)
{
    if (CurrentSort == InMode) return;
    CurrentSort = InMode;
    ApplySort();
    ListView->RequestListRefresh();
}

void STextureBudgetWindow::ApplySort()
{
    switch (CurrentSort)
    {
    case ESortMode::Ascending:
        RowData.Sort([](const TSharedPtr<FTextureFootprint>& A,
            const TSharedPtr<FTextureFootprint>& B)
            { return A->ResourceSizeBytes < B->ResourceSizeBytes; });
        break;

    case ESortMode::Descending:
        RowData.Sort([](const TSharedPtr<FTextureFootprint>& A,
            const TSharedPtr<FTextureFootprint>& B)
            { return A->ResourceSizeBytes > B->ResourceSizeBytes; });
        break;

    case ESortMode::Alphabetical:
        RowData.Sort([](const TSharedPtr<FTextureFootprint>& A,
            const TSharedPtr<FTextureFootprint>& B)
            { return A->AssetData.AssetName.LexicalLess(B->AssetData.AssetName); });
        break;
    }
}

FText STextureBudgetWindow::GetSortLabel() const
{
    switch (CurrentSort)
    {
    case ESortMode::Ascending:    return FText::FromString(TEXT("Ascending"));
    case ESortMode::Descending:   return FText::FromString(TEXT("Descending"));
    case ESortMode::Alphabetical: return FText::FromString(TEXT("Alphabetical"));
    default:                      return FText::FromString(TEXT("Sort"));
    }
}

/*-------------------------- Abrir Asset ---------------------------------*/
void STextureBudgetWindow::OpenAsset(const TSharedPtr<FTextureFootprint>& Item)
{
    if (!Item.IsValid()) return;

    if (UAssetEditorSubsystem* Sub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
    {
        if (UObject* Asset = Item->AssetData.GetAsset())
            Sub->OpenEditorForAsset(Asset);
    }
}
#endif /* WITH_EDITOR */
