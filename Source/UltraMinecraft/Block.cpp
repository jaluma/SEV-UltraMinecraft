// Fill out your copyright notice in the Description page of Project Settings.

#include "Block.h"
#include "UltraMinecraftCharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ABlock::ABlock()
{
	SM_Block = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block Mesh"));
	SetRootComponent(SM_Block);

	CollisionMesh = CreateDefaultSubobject<UBoxComponent>(FName("Collision Mesh"));
	FVector Scale;
	Scale.X = 1.6;
	Scale.Y = 1.6;
	Scale.Z = 1.6;
	CollisionMesh->SetRelativeScale3D(Scale);

	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	static ConstructorHelpers::FObjectFinder<USoundCue> breakSound(TEXT("/Game/Assets/Sound/Effect/Break/Break_Cue.Break_Cue"));
	BreakSound = breakSound.Object;

	//// check collision mesh to relative positio.
	//FVector Vector = FVector(0, 0, 0);

	//CollisionMesh->SetRelativeTransform(FTransform(Vector));

	// Replace in Blueprint class
	Resistance = 20.f;
	BreakingStage = 0.f;
	MinimumMaterial = 0;
	RequiredType = 0;
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
		// check if character using minimum tool
		AUltraMinecraftCharacter* Character = Cast<AUltraMinecraftCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		uint8 ToolMaterial = Character->ToolMaterial;
		uint8 ToolType = Character->ToolType;

		bool HasRequiredTool = ToolMaterial >= MinimumMaterial;

		if (RequiredType != 0) {
			HasRequiredTool = HasRequiredTool && ToolType == RequiredType;
		}

		OnBroken(HasRequiredTool);
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

void ABlock::OnBroken(bool HasRequiredTool)
{
	FVector SpawnLocation = GetActorLocation();
	TSubclassOf<class AWieldable> ClassType = WieldableType;

	if (Destroy() && ClassType != NULL && HasRequiredTool) {
		PlaySound(SpawnLocation);
		GetWorld()->SpawnActor<AWieldable>(ClassType, SpawnLocation, FRotator::ZeroRotator);
	}
}

void ABlock::PlaySound(FVector SpawnLocation)
{
	// try and play the sound if specified
	if (BreakSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, SpawnLocation);
	}
}
