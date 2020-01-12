// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Block.generated.h"

UCLASS()
class ULTRAMINECRAFT_API ABlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlock();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* SM_Block;

	uint8 MinimumMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Resistance;

	UPROPERTY(BlueprintReadWrite)
	float BreakingStage;

	void Break();

	void ResetBlock();

	void OnBroken(bool HasRequiredTool, ANSICHAR* typeTool);
};
