#include "STextureBudgetWindow.h"
#if WITH_EDITOR

#include "TextureBudgetScanner.h"
#include "AssetThumbnail.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"          // SVerticalBox / SHorizontalBox
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

TSharedPtr<FAssetThumbnailPool> STextureBudgetWindow::ThumbPool;

void STextureBudgetWindow::Construct(const FArguments& InArgs)
{
    if (!ThumbPool.IsValid())
    {
        ThumbPool = MakeShareable(new FAssetThumbnailPool(32));
    }

    /* Cria scanner e callback */
    Scanner = NewObject<UTextureBudgetScanner>();
    Scanner->AddToRoot();
    Scanner->OnScanFinished.AddRaw(this, &STextureBudgetWindow::HandleScanFinished);

    /* ------------------ UI ------------------ */
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

                /* ListView com thumbnails */
                + SVerticalBox::Slot().FillHeight(1.f).Padding(4)
                [
                    SAssignNew(ListView, SListView<TSharedPtr<FTextureFootprint>>)
                        .ListItemsSource(&RowData)

                        /* gera cada linha */
                        .OnGenerateRow_Lambda([](TSharedPtr<FTextureFootprint> Item,
                            const TSharedRef<STableViewBase>& Owner)
                            {
                                TSharedPtr<FAssetThumbnail> Thumb = MakeShareable(
                                    new FAssetThumbnail(Item->AssetData, 64, 64, STextureBudgetWindow::ThumbPool));

                                return SNew(STableRow<TSharedPtr<FTextureFootprint>>, Owner)
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

                        /* duplo-clique → abrir no editor */
                        .OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FTextureFootprint> Item)
                            {
                                OpenAsset(Item);
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

/* ---------------------------------------------------
   Abre a textura na aba padrão de edição
   --------------------------------------------------- */
void STextureBudgetWindow::OpenAsset(const TSharedPtr<FTextureFootprint>& Item)
{
    if (!Item.IsValid()) return;

    UObject* Asset = Item->AssetData.GetAsset();
    if (!Asset) return;

    if (UAssetEditorSubsystem* Sub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
    {
        Sub->OpenEditorForAsset(Asset);
    }
}

#endif  // WITH_EDITOR
