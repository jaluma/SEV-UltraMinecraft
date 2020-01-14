// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "ItemCrafting.h"
#include "UltraMinecraftGameMode.generated.h"

UCLASS(minimalapi)
class AUltraMinecraftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUltraMinecraftGameMode();

	enum EHUDState : uint8 {
		HS_Ingame,
		HS_Inventory,
		HS_Craft_Menu,
	};

	/* Checks the hud state, and then calls applyhud to apply whatever hud we are using */
	void ApplyHUDChange();

	uint8 GetHUDState();

	/* Setter functionfor hudstate, applies the new states and then calls ApplyHUDChange */
	UFUNCTION(BlueprintCallable, Category = "HUD Functions")
	void ChangeHUDState(uint8 NewState);

	/* Applies a hud to the screen, returns true if successful, false otherwise */
	bool ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool ShowMouseCursor, bool EnableClickEvents);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FItemCrafting> GetAvailableCrafting();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	FItemCrafting GetCrafting(FItemCrafting itemInfo);

protected:
	/* Current HUD State */
	uint8 HUDState;

	/* The hud to be shown ingame */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = true))
	TSubclassOf<class UUserWidget> IngameHUDClass;

	/* The hud to be shown in the inventory */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = true))
	TSubclassOf<class UUserWidget> InventoryHUDClass;

	/* The hud to be shown in the crafting menu */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Blueprint Widgets", Meta = (BlueprintProtected = true))
	TSubclassOf<class UUserWidget> CraftMenuHUDClass;

	UFUNCTION(BlueprintPure, Category = Widgets)
	class UUserWidget* GetCurrentWidget();

	/* The current hud being displayed on the screen */
	class UUserWidget* CurrentWidget;

	UDataTable* CraftingDataTable;
};



