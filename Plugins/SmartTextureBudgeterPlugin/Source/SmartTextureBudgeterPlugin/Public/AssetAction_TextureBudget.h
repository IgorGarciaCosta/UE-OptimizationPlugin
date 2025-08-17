#pragma once
#if WITH_EDITOR

#include "AssetTypeActions_Base.h"
#include "Engine/Texture2D.h"

class FAssetAction_TextureBudget : public FAssetTypeActions_Base
{
public:
    virtual FText GetName() const override
    {
        return NSLOCTEXT("AssetActions", "TextureBudget", "Optimize Texture for Budget");
    }

    virtual UClass* GetSupportedClass() const override
    {
        return UTexture2D::StaticClass();
    }

    virtual uint32 GetCategories() override
    {
        return EAssetTypeCategories::Textures;
    }
};

#endif
