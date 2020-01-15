// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine.h"
#include "Wieldable.generated.h"

UCLASS()
class ULTRAMINECRAFT_API AWieldable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWieldable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	 
	enum ETool : uint8
	{
		Unarmed = 0,
		Block = 1,
		Pickaxe = 2,
		Axe = 3,
		Shovel = 4,
		Sword = 5,

	};

	enum EMaterial : uint8
	{
		None = 1,
		Wooden = 2,
		Stone = 4,
		Iron = 6,
		Golden = 9,
		Diamond = 12
	};

	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere)
	uint8 ToolType;

	UPROPERTY(EditAnywhere)
	uint8 MaterialType;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* WieldableMesh;

	UPROPERTY(EditAnywhere)
	UShapeComponent* PickupTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	UTexture2D* PickupThumbnail;

	//it's AActor class, NOT BLOCK
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> BlockType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ClassName;

	bool IsActive;

	UFUNCTION()
	void OnRadiusEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Hide(bool bVis);

	void OnUsed();
};
