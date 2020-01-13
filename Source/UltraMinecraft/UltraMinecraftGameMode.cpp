// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UltraMinecraftGameMode.h"
#include "UltraMinecraftHUD.h"
#include "UltraMinecraftCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AUltraMinecraftGameMode::AUltraMinecraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUltraMinecraftHUD::StaticClass();

	HUDState = EHUDState::HS_Ingame;
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
