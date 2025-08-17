#include "TextureBudgetStyle.h"
#if WITH_EDITOR
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyle.h"

static TSharedPtr<FSlateStyleSet> StyleInstance;

void FTextureBudgetStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = MakeShareable(new FSlateStyleSet("TextureBudgetStyle"));
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FTextureBudgetStyle::Shutdown()
{
    if (StyleInstance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
        StyleInstance.Reset();
    }
}
#endif
