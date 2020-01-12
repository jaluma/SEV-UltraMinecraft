// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UltraMinecraftGameMode.h"
#include "UltraMinecraftHUD.h"
#include "UltraMinecraftCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUltraMinecraftGameMode::AUltraMinecraftGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUltraMinecraftHUD::StaticClass();
}
