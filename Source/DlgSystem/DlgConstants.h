// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

static const FName DIALOGUE_SYSTEM_MENU_CATEGORY_KEY(TEXT("Dialogue System"));
static const FText DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT(NSLOCTEXT("DlgSystemEditor", "DlgSystemAssetCategory", "Dialogue System"));

static const FName OTHER_DIALOGUE_SYSTEM_MENU_CATEGORY_KEY(TEXT("Other Dialogue System"));
static const FText OTHER_DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT(NSLOCTEXT("OtherDlgSystemEditor", "OtherDlgSystemAssetCategory", "OtherDialogue System"));

// DlgDataDisplay TabID
static const FName DIALOGUE_DATA_DISPLAY_TAB_ID(TEXT("DlgDataDisplayWindow"));

// Other Modules constants
static const FName NAME_MODULE_AssetTools(TEXT("AssetTools"));
static const FName NAME_MODULE_AssetRegistry(TEXT("AssetRegistry"));
static const FName NAME_MODULE_LevelEditor(TEXT("LevelEditor"));
static const FName NAME_MODULE_PropertyEditor(TEXT("PropertyEditor"));

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dlg);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dlg_Cat);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dlg_Frog);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dlg_Hero);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dlg_Human);
