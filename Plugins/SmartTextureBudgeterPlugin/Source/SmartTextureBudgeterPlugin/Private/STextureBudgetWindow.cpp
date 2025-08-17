#include "STextureBudgetWindow.h"
#if WITH_EDITOR

#include "TextureBudgetScanner.h"
#include "AssetThumbnail.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h" 
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

TSharedPtr<FAssetThumbnailPool> STextureBudgetWindow::ThumbPool;

void STextureBudgetWindow::Construct(const FArguments& InArgs)
{
    /* Pool único (compartilhado) */
    if (!ThumbPool.IsValid())
    {
        ThumbPool = MakeShareable(new FAssetThumbnailPool(32));
    }

    /* Cria o scanner e liga callback */
    Scanner = NewObject<UTextureBudgetScanner>();
    Scanner->AddToRoot();
    Scanner->OnScanFinished.AddRaw(this, &STextureBudgetWindow::HandleScanFinished);

    /* ---------- UI ---------- */
    ChildSlot
        [
            SNew(SVerticalBox)

                /* Botão Scan */
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Scan Project")))
                        .OnClicked(this, &STextureBudgetWindow::OnScanClicked)
                ]

                /* ListView */
                + SVerticalBox::Slot().FillHeight(1.f).Padding(4)
                [
                    SAssignNew(ListView, SListView<TSharedPtr<FTextureFootprint>>)
                        .ListItemsSource(&RowData)

                        .OnGenerateRow_Lambda([](TSharedPtr<FTextureFootprint> Item,
                            const TSharedRef<STableViewBase>& OwnerTable)
                            {
                                /* Thumbnail 64×64 */
                                TSharedPtr<FAssetThumbnail> Thumb = MakeShareable(
                                    new FAssetThumbnail(Item->AssetData, 64, 64, STextureBudgetWindow::ThumbPool));

                                return SNew(STableRow<TSharedPtr<FTextureFootprint>>, OwnerTable)
                                    [
                                        SNew(SHorizontalBox)

                                            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                            [
                                                Thumb->MakeThumbnailWidget()
                                            ]

                                            + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0)
                                            [
                                                SNew(STextBlock)
                                                    .Text(FText::FromName(Item->AssetData.AssetName))
                                            ]
                                    ];
                            })
                ]
        ];
}

FReply STextureBudgetWindow::OnScanClicked()
{
    if (Scanner)
    {
        RowData.Empty();
        ListView->RequestListRefresh();
        Scanner->ScanProject();
    }
    return FReply::Handled();
}

void STextureBudgetWindow::HandleScanFinished(const TArray<FTextureFootprint>& Results)
{
    RowData.Empty();
    for (const FTextureFootprint& FP : Results)
    {
        RowData.Add(MakeShared<FTextureFootprint>(FP));
    }
    ListView->RequestListRefresh();
}

#endif  // WITH_EDITOR
