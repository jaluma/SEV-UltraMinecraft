// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wieldable.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* CollisionMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* SM_Block;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 MinimumMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 RequiredType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Resistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AWieldable> WieldableType;

	UPROPERTY(BlueprintReadWrite)
	float BreakingStage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* BreakSound;

	void Break();

	void ResetBlock();

	void OnBroken(bool HasRequiredTool);

	void PlaySound(FVector SpawnLocation);
};
