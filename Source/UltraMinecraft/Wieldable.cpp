// Fill out your copyright notice in the Description page of Project Settings.

#include "Wieldable.h"
#include "Components/BoxComponent.h"
#include "UltraMinecraftCharacter.h"


// Sets default values
AWieldable::AWieldable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WieldableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WieldableMesh"));

	PickupTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupTrigger"));

	PickupTrigger->bGenerateOverlapEvents = true;
	PickupTrigger->OnComponentBeginOverlap.AddDynamic(this, &AWieldable::OnRadiusEnter);
	PickupTrigger->AttachToComponent(WieldableMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("FP_WieldItem"));

	MaterialType = EMaterial::None;
	ToolType = ETool::Unarmed;
	IsActive = true;
}

// Called when the game starts or when spawned
void AWieldable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWieldable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator rotation = WieldableMesh->GetComponentRotation();
	rotation.Yaw += 1.0f;
	WieldableMesh->SetRelativeRotation(rotation);
}

void AWieldable::OnRadiusEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsActive) {
		AUltraMinecraftCharacter* Character = Cast<AUltraMinecraftCharacter>(OtherActor);
		if (Character != nullptr) {
			Character->AddItemToInventory(this);

			//Hide(true);
			OnUsed();
		}
	}
}

void AWieldable::Hide(bool bVis)
{
	WieldableMesh->SetVisibility(!bVis);
	IsActive = !bVis;
}

void AWieldable::OnUsed()
{
	Destroy();
}

