// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UltraMinecraftGameMode.h"
#include "UltraMinecraftHUD.h"
#include "UltraMinecraftCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "ItemCrafting.h"
#include "Kismet/GameplayStatics.h"

AUltraMinecraftGameMode::AUltraMinecraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUltraMinecraftHUD::StaticClass();

	HUDState = EHUDState::HS_Ingame;

	// init data crafting table
	static ConstructorHelpers::FObjectFinder<UDataTable>
		CraftingDataTable_BP(TEXT("DataTable'/Game/Assets/Crafting/crafting.crafting'"));
	CraftingDataTable = CraftingDataTable_BP.Object;
}

void AUltraMinecraftGameMode::ApplyHUDChange()
{
	if (CurrentWidget != nullptr) {
		CurrentWidget->RemoveFromParent();
	}

	switch (HUDState) {
		case EHUDState::HS_Ingame: {
			ApplyHUD(IngameHUDClass, false, false);
			break;
		}
		case EHUDState::HS_Inventory: {
			ApplyHUD(InventoryHUDClass, true, true);
			break;
		}
		case EHUDState::HS_Craft_Menu: {
			ApplyHUD(CraftMenuHUDClass, false, false);
			break;
		}
		default: {
			ApplyHUD(IngameHUDClass, false, false);
			break;
		}
	}
}

uint8 AUltraMinecraftGameMode::GetHUDState()
{
	return HUDState;
}

void AUltraMinecraftGameMode::ChangeHUDState(uint8 NewState)
{
	HUDState = NewState;
	ApplyHUDChange();
}

bool AUltraMinecraftGameMode::ApplyHUD(TSubclassOf<class UUserWidget> WidgetToApply, bool ShowMouseCursor, bool EnableClickEvents)
{
	AUltraMinecraftCharacter* myChracter = Cast<AUltraMinecraftCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();

	if (WidgetToApply != nullptr) {
		MyController->bShowMouseCursor = ShowMouseCursor;
		MyController->bEnableClickEvents = EnableClickEvents;

		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToApply);

		if (CurrentWidget != nullptr) {
			CurrentWidget->AddToViewport();
			CurrentWidget->SetKeyboardFocus();
			return true;
		}
	}

	return false;
}

UUserWidget * AUltraMinecraftGameMode::GetCurrentWidget()
{
	return CurrentWidget;
}

TArray<FItemCrafting> AUltraMinecraftGameMode::GetAvailableCrafting()
{
	TArray<FItemCrafting> CraftingArray;

	for (auto& item : CraftingDataTable->RowMap)
	{
		CraftingArray.Add(*(FItemCrafting*)(item.Value));
	}
	return CraftingArray;
}

AActor* AUltraMinecraftGameMode::SpawnBlueprintFromPath(UWorld* MyWorld, const FString PathToBlueprint, const FVector SpawnLocation, FRotator SpawnRotation)
{
	FStringAssetReference ItemToReference(PathToBlueprint);
	UObject* ItemObject = ItemToReference.ResolveObject();
	if ((ItemObject) && (MyWorld))
	{
		UBlueprint* GeneratedBP = Cast<UBlueprint>(ItemObject);
		return MyWorld->SpawnActor<AActor>(GeneratedBP->GeneratedClass, SpawnLocation, SpawnRotation);
	}
	else {
		return NULL;
	}
}
