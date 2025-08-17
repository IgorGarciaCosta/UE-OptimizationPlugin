#include "STextureBudgetWindow.h"
#if WITH_EDITOR

#include "TextureBudgetScanner.h"
#include "AssetThumbnail.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SOverlay.h"

TSharedPtr<FAssetThumbnailPool> STextureBudgetWindow::ThumbPool;

/*------------------------------------------------------------*/
void STextureBudgetWindow::Construct(const FArguments& InArgs)
{
    if (!ThumbPool.IsValid())
        ThumbPool = MakeShareable(new FAssetThumbnailPool(32));

    /* Scanner */
    Scanner = NewObject<UTextureBudgetScanner>();
    Scanner->AddToRoot();
    Scanner->OnFootprintReady.AddRaw(this, &STextureBudgetWindow::HandleFootprintReady);
    Scanner->OnFinished.AddRaw(this, &STextureBudgetWindow::HandleScanFinished);

    /* UI --------------------------------------------------- */
    ChildSlot
        [
            SNew(SOverlay)

                /* camada 0 – conteúdo */
                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                        + SVerticalBox::Slot().AutoHeight().Padding(4)
                        [
                            SNew(SButton)
                                .Text(FText::FromString(TEXT("Scan Project")))
                                .OnClicked(this, &STextureBudgetWindow::OnScanClicked)
                                .IsEnabled_Lambda([this]() { return !bIsScanning; })
                        ]

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

                /* camada 1 – spinner */
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

/*------------------------------------------------------------*/
FReply STextureBudgetWindow::OnScanClicked()
{
    if (bIsScanning) return FReply::Handled();

    bIsScanning = true;
    RowData.Empty();
    ListView->RequestListRefresh();
    LastRefresh = FPlatformTime::Seconds();

    Scanner->StartAsyncScan(32);    // lote de 32

    return FReply::Handled();
}

/*------------------------------------------------------------*/
void STextureBudgetWindow::HandleFootprintReady(const FTextureFootprint& FP)
{
    RowData.Add(MakeShared<FTextureFootprint>(FP));

    const double Now = FPlatformTime::Seconds();
    if (Now - LastRefresh > 0.25)   // evita refresh excessivo
    {
        LastRefresh = Now;
        ListView->RequestListRefresh();
    }
}

/*------------------------------------------------------------*/
void STextureBudgetWindow::HandleScanFinished()
{
    ListView->RequestListRefresh();   // refresh final
    bIsScanning = false;
}

/*------------------------------------------------------------*/
void STextureBudgetWindow::OpenAsset(const TSharedPtr<FTextureFootprint>& Item)
{
    if (!Item.IsValid()) return;

    if (UAssetEditorSubsystem* Sub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
    {
        if (UObject* Asset = Item->AssetData.GetAsset())
        {
            Sub->OpenEditorForAsset(Asset);
        }
    }
}
#endif /* WITH_EDITOR */
