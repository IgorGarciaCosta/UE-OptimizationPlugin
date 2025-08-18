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
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SOverlay.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"



TSharedPtr<FAssetThumbnailPool> STextureBudgetWindow::ThumbPool;

/*------------------------------------------------------------------------*/
void STextureBudgetWindow::Construct(const FArguments& InArgs)
{
    if (!ThumbPool.IsValid())
        ThumbPool = MakeShareable(new FAssetThumbnailPool(32));

    /* Scanner -------------------------------------------------- */
    Scanner = NewObject<UTextureBudgetScanner>();
    Scanner->AddToRoot();
    Scanner->OnFootprintReady.AddRaw(this, &STextureBudgetWindow::HandleFootprintReady);
    Scanner->OnFinished.AddRaw(this, &STextureBudgetWindow::HandleScanFinished);

    /* UI ------------------------------------------------------- */
    ChildSlot
        [
            SNew(SOverlay)

                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                        /* --- Barra de botões -------------------------------- */
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

                                /* Search */
                                + SHorizontalBox::Slot().FillWidth(1.f)
                                [
                                    SAssignNew(SearchBox, SSearchBox)
                                        .HintText(FText::FromString(TEXT("Search...")))
                                        .OnTextChanged(this, &STextureBudgetWindow::OnSearchChanged)
                                ]
                        ]

                        /* --- Lista ------------------------------------------ */
                        + SVerticalBox::Slot().FillHeight(1.f).Padding(4)
                        [
                            SAssignNew(ListView, SListView<TSharedPtr<FTextureFootprint>>)
                                .ListItemsSource(&RowData)

                                .OnGenerateRow_Lambda([this](TSharedPtr<FTextureFootprint> Item,
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
                                            Item->Width, Item->Height, SizeMB);

                                        return SNew(STableRow<TSharedPtr<FTextureFootprint>>, Owner)
                                            [
                                                SNew(SHorizontalBox)

                                                    /* thumbnail */
                                                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                                                    [
                                                        Thumb->MakeThumbnailWidget()
                                                    ]

                                                    /* texto */
                                                    + SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8, 0)
                                                    [
                                                        SNew(STextBlock).Text(FText::FromString(Label))
                                                    ]

                                                    /* botão Resize */
                                                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
                                                    [
                                                        SNew(SButton)
                                                            .Text(FText::FromString(TEXT("Resize")))
                                                            .OnClicked_Lambda([this, Item]()
                                                                {
                                                                    OpenResizeDialog(Item);
                                                                    return FReply::Handled();
                                                                })
                                                    ]
                                            ];
                                    })

                                .OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FTextureFootprint> Item)
                                    {
                                        OpenAsset(Item);
                                    })
                        ]
                ]

                /* --- Spinner ------------------------------------------- */
                + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SAssignNew(Spinner, SThrobber)
                        .Visibility_Lambda([this]() { return bIsScanning ? EVisibility::Visible : EVisibility::Collapsed; })
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

    auto AddEntry = [&](ESortMode Mode, const TCHAR* Label)
        {
            Menu.AddMenuEntry(
                FText::FromString(Label), FText::GetEmpty(), FSlateIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &STextureBudgetWindow::OnSortChosen, Mode)));
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
        RowData.Sort([](const auto& A, const auto& B) { return A->ResourceSizeBytes < B->ResourceSizeBytes; });
        break;
    case ESortMode::Descending:
        RowData.Sort([](const auto& A, const auto& B) { return A->ResourceSizeBytes > B->ResourceSizeBytes; });
        break;
    case ESortMode::Alphabetical:
        RowData.Sort([](const auto& A, const auto& B) { return A->AssetData.AssetName.LexicalLess(B->AssetData.AssetName); });
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

void STextureBudgetWindow::OpenResizeDialog(
    const TSharedPtr<FTextureFootprint>& Item)
{
    if (!Item.IsValid()) return;
    UTexture2D* Tex = Cast<UTexture2D>(Item->AssetData.GetAsset());
    if (!Tex) return;

    /* fecha pop-up anterior */
    if (ActiveResizeWindow.IsValid())
    {
        ActiveResizeWindow.Pin()->RequestDestroyWindow();
        ActiveResizeWindow.Reset();
    }

    /* valores iniciais */
    TSharedRef<int32> NewW = MakeShared<int32>(Item->Width);
    TSharedRef<int32> NewH = MakeShared<int32>(Item->Height);

    TSharedRef<SWindow> Win = SNew(SWindow)
        .Title(FText::FromString(TEXT("Resize Texture")))
        .ClientSize(FVector2D(240, 110))
        .SupportsMaximize(false)
        .SupportsMinimize(false);

    ActiveResizeWindow = Win;

    Win->SetContent
    (
        SNew(SVerticalBox)

        /* -------- X -------- */
        + SVerticalBox::Slot().AutoHeight().Padding(6)
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)
                [SNew(STextBlock).Text(FText::FromString(TEXT("X")))]
                + SHorizontalBox::Slot().FillWidth(1.f)
                [
                    SNew(SNumericEntryBox<int32>)
                        .Value_Lambda([NewW]() { return TOptional<int32>(*NewW); })
                        .OnValueChanged_Lambda([NewW](int32 Val) { *NewW = Val; })
                ]
        ]

        /* -------- Y -------- */
        + SVerticalBox::Slot().AutoHeight().Padding(6, 0, 6, 6)
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)
                [SNew(STextBlock).Text(FText::FromString(TEXT("Y")))]
                + SHorizontalBox::Slot().FillWidth(1.f)
                [
                    SNew(SNumericEntryBox<int32>)
                        .Value_Lambda([NewH]() { return TOptional<int32>(*NewH); })
                        .OnValueChanged_Lambda([NewH](int32 Val) { *NewH = Val; })
                ]
        ]

        /* -------- OK -------- */
        + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(6)
        [
            SNew(SButton)
                .Text(FText::FromString(TEXT("OK")))
                .OnClicked_Lambda([this, Win, Tex, Item, NewW, NewH]()
                    {
                        /* aplica o valor digitado */
                        Tex->Modify();
                        Tex->MaxTextureSize = FMath::Max(*NewW, *NewH);
                        Tex->PostEditChange();
                        Tex->UpdateResource();                // força recriar mips
                        Tex->MarkPackageDirty();

                        /* mostra exatamente o que o usuário colocou */
                        Item->Width = *NewW;
                        Item->Height = *NewH;
                        Item->ResourceSizeBytes =
                            Tex->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
                        ListView->RequestListRefresh();

                        Win->RequestDestroyWindow();
                        ActiveResizeWindow.Reset();
                        return FReply::Handled();
                    })
        ]
    );

    FSlateApplication::Get().AddWindow(Win);
}



/*------------------------ Abrir asset -----------------------------------*/
void STextureBudgetWindow::OpenAsset(const TSharedPtr<FTextureFootprint>& Item)
{
    if (!Item.IsValid()) return;
    if (UAssetEditorSubsystem* Sub = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
        if (UObject* Asset = Item->AssetData.GetAsset())
            Sub->OpenEditorForAsset(Asset);
}
#endif /* WITH_EDITOR */
