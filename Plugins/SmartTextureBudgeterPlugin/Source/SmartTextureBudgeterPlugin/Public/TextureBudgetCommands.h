#pragma once
#if WITH_EDITOR

#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FTextureBudgetCommands : public TCommands<FTextureBudgetCommands>
{
public:
    FTextureBudgetCommands()
        : TCommands<FTextureBudgetCommands>(
            TEXT("TextureBudget"),
            NSLOCTEXT("Contexts", "TextureBudget", "Texture Budget"),
            NAME_None,
            FEditorStyle::GetStyleSetName())
    {}

    virtual void RegisterCommands() override;
};

#endif  // WITH_EDITOR
