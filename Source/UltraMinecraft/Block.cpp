// Fill out your copyright notice in the Description page of Project Settings.

#include "Block.h"


// Sets default values
ABlock::ABlock()
{
	SM_Block = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block Mesh"));

	Resistance = 20.f;
	BreakingStage = 0.f;
	MinimumMaterial = 0;
}

// Called when the game starts or when spawned
void ABlock::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlock::Break()
{
	++BreakingStage;
	float CrackingValue = 1.0f - (BreakingStage / 5.f);

	UMaterialInstanceDynamic* MatInstance = SM_Block->CreateDynamicMaterialInstance(0);

	if (MatInstance != nullptr) {
		MatInstance->SetScalarParameterValue(FName("CrackingValue"), CrackingValue);
	}

	// it's broken
	if (BreakingStage == 5.f) {
		OnBroken(true, "Pichaxe");
	}
}

void ABlock::ResetBlock()
{
	BreakingStage = 0.f;
	UMaterialInstanceDynamic* MatInstance = SM_Block->CreateDynamicMaterialInstance(0);

	if (MatInstance != nullptr) {
		MatInstance->SetScalarParameterValue(FName("CrackingValue"), 1.0f);
	}
}

void ABlock::OnBroken(bool HasRequiredTool, ANSICHAR * typeTool)
{
	Destroy();
}
