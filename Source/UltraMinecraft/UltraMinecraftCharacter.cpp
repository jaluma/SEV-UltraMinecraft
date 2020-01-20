// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "UltraMinecraftCharacter.h"
#include "UltraMinecraftProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "UltraMinecraftGameMode.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AUltraMinecraftCharacter

AUltraMinecraftCharacter::AUltraMinecraftCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_ItemRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FP_ItemRight"));
	FP_ItemRight->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_ItemRight->bCastDynamicShadow = false;
	FP_ItemRight->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_ItemRight->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_ItemRight);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	bUsingMotionControllers = false;

	Inventory.Init(nullptr, NUM_OF_INVENTORY_SLOTS);
	CraftingInventory.Init(nullptr, NUM_OF_CRAFTING_INVENTORY_SLOTS);

	Reach = 400;
	InitialPlayerHealth = 10;
	PlayerHealth = InitialPlayerHealth;

	static ConstructorHelpers::FObjectFinder<USoundCue> hitSound(TEXT("/Game/Assets/Sound/Effect/Hit/Hit_Cue.Hit_Cue"));
	HitSound = hitSound.Object;

	static ConstructorHelpers::FObjectFinder<USoundCue> putSound(TEXT("/Game/Assets/Sound/Effect/Put/Put_Cue.Put_Cue"));
	PutSound = putSound.Object;
}

void AUltraMinecraftCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_ItemRight->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	AUltraMinecraftGameMode * mymode = Cast<AUltraMinecraftGameMode>(GetWorld()->GetAuthGameMode());
	mymode->ApplyHUDChange();

	UpdateWieldedItem();
}

void AUltraMinecraftCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForBlocks();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUltraMinecraftCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AUltraMinecraftCharacter::Throw);

	PlayerInputComponent->BindAction("PutBlocks", IE_Pressed, this, &AUltraMinecraftCharacter::Put);
	
	PlayerInputComponent->BindAction("Inventary", IE_Pressed, this, &AUltraMinecraftCharacter::ShowHideCrafting);
	PlayerInputComponent->BindAction("InventoryDown", IE_Pressed, this, &AUltraMinecraftCharacter::MoveDownInventorySlot);
	PlayerInputComponent->BindAction("InventoryUp", IE_Pressed, this, &AUltraMinecraftCharacter::MoveUpInventorySlot);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AUltraMinecraftCharacter::TouchStarted);
	if (EnableTouchscreenMovement(PlayerInputComponent) == false)
	{
		PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AUltraMinecraftCharacter::OnHit);
		PlayerInputComponent->BindAction("Interact", IE_Released, this, &AUltraMinecraftCharacter::EndHit);
	}

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUltraMinecraftCharacter::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUltraMinecraftCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUltraMinecraftCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AUltraMinecraftCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AUltraMinecraftCharacter::LookUpAtRate);
}

int32 AUltraMinecraftCharacter::GetCurrentInventorySlot()
{
	return CurrentInventorySlot;
}

AWieldable* AUltraMinecraftCharacter::GetItemByIndex(int32 Index)
{ 
	if (Index >= 0 && Index < NUM_OF_INVENTORY_SLOTS) {
		return Inventory[Index];
	}
	else {
		return nullptr;
	}
}

bool AUltraMinecraftCharacter::AddItemToInventory(AWieldable * Item)
{
	if (Item != nullptr) {
		// CHECK if available slot
		bool isAvailable = false;
		for (int i = 0; i < NUM_OF_INVENTORY_SLOTS; i++) {
			if (Inventory[i] == nullptr) {
				isAvailable = true;
				break;
			}
		}
		if (isAvailable) {
			const int32 AvailableSlot = Inventory.Find(nullptr);
			if (AvailableSlot != INDEX_NONE) {
				Inventory[AvailableSlot] = Item;
				Item->Hide(true);

				// Update mesh if it's change
				if (Inventory[CurrentInventorySlot] != NULL && Inventory[CurrentInventorySlot]->WieldableMesh != nullptr) {
					UpdateWieldedItem();
				}
				return true;
			}
		}
 		
	}
	return false;
}

bool AUltraMinecraftCharacter::HaveWieldableItemInInventory(TSubclassOf<class AWieldable> classType)
{
	if (classType != NULL) {
		for (int i = 0; i < NUM_OF_INVENTORY_SLOTS; i++) {
			if (Inventory[i] != nullptr && Inventory[i]->GetClass() == classType.Get()) {
				return true;
			}
		}
	}
	return false;
}

bool AUltraMinecraftCharacter::AddItemToCraftingTableAt(int32 Index)
{
	if (Index >= 0 && Index < NUM_OF_CRAFTING_INVENTORY_SLOTS) {
		AddItemToCraft(Index);
		return true;
	}
	return false;
}

void AUltraMinecraftCharacter::GetCraftingItem()
{
	// Try to add item to inventory, if Inventory full -> Throw item
	if (!AddItemToInventory(CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1])) {
		Throw(CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1]);
	}
	// reset crafting table
	CraftingInventory.Init(nullptr, NUM_OF_CRAFTING_INVENTORY_SLOTS);
}

void AUltraMinecraftCharacter::AddItemToCraft(int32 Index) {
	if (CraftingInventory[Index] == nullptr) {
		CraftingInventory[Index] = Inventory[CurrentInventorySlot];
		Inventory[CurrentInventorySlot] = nullptr;
	}
	else {
		// swap crafting item by inventory item
		AWieldable* Temp = Inventory[CurrentInventorySlot];
		Inventory[CurrentInventorySlot] = CraftingInventory[Index];
		CraftingInventory[Index] = Temp;
	}
	UpdateWieldedItem();
	UpdatePossiblyCraft();
}

void AUltraMinecraftCharacter::UpdatePossiblyCraft()
{
	AUltraMinecraftGameMode * mymode = Cast<AUltraMinecraftGameMode>(GetWorld()->GetAuthGameMode());
	for (FItemCrafting c : mymode->GetAvailableCrafting()) {
		if (
			
			CheckCraftCorrect(0, c.Item1) && CheckCraftCorrect(1, c.Item2) &&
			CheckCraftCorrect(2, c.Item3) && CheckCraftCorrect(3, c.Item4) &&
			CheckCraftCorrect(4, c.Item5) && CheckCraftCorrect(5, c.Item6) &&
			CheckCraftCorrect(6, c.Item7) && CheckCraftCorrect(7, c.Item8) &&
			CheckCraftCorrect(8, c.Item9)
			) {

			// destroy before crafted item if it wasn't selected
			if (CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1] != nullptr) {
				CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1]->Destroy();
			}

			AWieldable* WieldableObject = Cast<AWieldable>(mymode->SpawnBlueprintFromPath(GetWorld(), c.Return, FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator));
			WieldableObject->IsActive = false;
			CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1] = WieldableObject;

			return;
		}
	}

	if (CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1] != nullptr) {
		CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1]->Destroy();
		CraftingInventory[NUM_OF_CRAFTING_INVENTORY_SLOTS - 1] = nullptr;
	}

	return;
}

bool AUltraMinecraftCharacter::CheckCraftCorrect(int32 Index, FString& Item)
{
	return (CraftingInventory[Index] == NULL && Item.IsEmpty()) || (CraftingInventory[Index] != NULL && Item.Equals(CraftingInventory[Index]->ClassName));
}

UTexture2D* AUltraMinecraftCharacter::GetThumnailAtInventorySlot(int32 Slot)
{
	if (Slot >= 0 && Slot < NUM_OF_INVENTORY_SLOTS && Inventory[Slot] != NULL) {
		return Inventory[Slot]->PickupThumbnail;
	}
	return nullptr;
}

UTexture2D* AUltraMinecraftCharacter::GetThumnailAtCraftingInventorySlot(int32 Slot)
{
	if (Slot >= 0 && Slot < NUM_OF_CRAFTING_INVENTORY_SLOTS && CraftingInventory[Slot] != NULL) {
		return CraftingInventory[Slot]->PickupThumbnail;
	}
	return nullptr;
}

bool AUltraMinecraftCharacter::DecPlayerHealth(int32 Dec)
{
	if (PlayerHealth - Dec >= -Dec) {
		PlayerHealth -= Dec;
		if (HitSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}
		return true;
	}
	return false;
}

int32 AUltraMinecraftCharacter::GetInitialHealth()
{
	return InitialPlayerHealth;
}

int32 AUltraMinecraftCharacter::GetHealth()
{
	return PlayerHealth;
}

bool AUltraMinecraftCharacter::IsHoldingWieldableItem(TSubclassOf<class AWieldable> classType)
{
	AWieldable* Wieldable = GetCurrentWieldedItem();

	if (classType == nullptr || Wieldable == nullptr) {
		return false;
	}

	if (Wieldable->GetClass() == classType.Get()) {
		return true;
	}

	return false;
}

void AUltraMinecraftCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<AUltraMinecraftProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<AUltraMinecraftProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AUltraMinecraftCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUltraMinecraftCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AUltraMinecraftCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AUltraMinecraftCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AUltraMinecraftCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AUltraMinecraftCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUltraMinecraftCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AUltraMinecraftCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AUltraMinecraftCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AUltraMinecraftCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AUltraMinecraftCharacter::TouchUpdate);
	}
	return bResult;
}

void AUltraMinecraftCharacter::UpdateWieldedItem()
{
	if (GetCurrentWieldedItem() != nullptr) {
		if (CopyMesh != nullptr) {
			CopyMesh->DestroyComponent();
		}
		//CopyMesh = Inventory[CurrentInventorySlot]->WieldableMesh;
		FP_ItemRight->SetStaticMesh(Inventory[CurrentInventorySlot]->WieldableMesh->GetStaticMesh());

		// it's a block
		if (Inventory[CurrentInventorySlot]->ToolType == 1) {
			FVector Relative3DScale;
			Relative3DScale.X = 0.2;
			Relative3DScale.Y = 0.2;
			Relative3DScale.Z = 0.2;
			FP_ItemRight->SetRelativeScale3D(Relative3DScale);
		}
		else {
			FVector Relative3DScale;
			Relative3DScale.X = 1;
			Relative3DScale.Y = 1;
			Relative3DScale.Z = 1;
			FP_ItemRight->SetRelativeScale3D(Relative3DScale);
		}

		// update tool material and type
		ToolType = Inventory[CurrentInventorySlot]->ToolType;
		ToolMaterial = Inventory[CurrentInventorySlot]->MaterialType;
	}
	else {
		FP_ItemRight->SetStaticMesh(NULL);

		ToolType = 0;
		ToolMaterial = 1;
	}
}

AWieldable * AUltraMinecraftCharacter::GetCurrentWieldedItem()
{
	return Inventory[CurrentInventorySlot] != NULL ? Inventory[CurrentInventorySlot] : nullptr;
}

void AUltraMinecraftCharacter::Throw()
{
	AWieldable* ItemToThrow = GetCurrentWieldedItem();
	Throw(ItemToThrow);
}

void AUltraMinecraftCharacter::Throw(AWieldable* ItemToThrow)
{
	FHitResult LinetraceHit;

	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * Reach) + StartTrace;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(LinetraceHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

	FVector DropLocation = EndTrace;
	if (LinetraceHit.GetActor() != NULL) {
		DropLocation = (LinetraceHit.ImpactPoint + 20.f);
	}

	if (ItemToThrow != nullptr) {
		UWorld* const World = GetWorld();
		if (World != NULL) {
			ItemToThrow->SetActorLocationAndRotation(DropLocation, FRotator::ZeroRotator);
			ItemToThrow->Hide(false); 

			Inventory[CurrentInventorySlot] = NULL;
			UpdateWieldedItem();
		}
	}
}

void AUltraMinecraftCharacter::Put()
{
	AWieldable* ItemToPut = GetCurrentWieldedItem();

	FHitResult LinetraceHit;

	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * Reach) + StartTrace;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(LinetraceHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

	if (LinetraceHit.GetActor() != NULL) {
		FVector Impact = (LinetraceHit.ImpactPoint);

		int x = abs(Impact.X);
		float Xmod = x % 100;
		int y = abs(Impact.Y);
		float Ymod = y % 100;
		int z = abs(Impact.Z);
		float Zmod = z % 100;

		float xRes = Xmod >= 50 ? x + (100 - Xmod) : x - Xmod;
		float yRes = Ymod >= 50 ? y + (100 - Ymod) : y - Ymod;
		float zRes = Zmod >= 50 ? z + (100 - Zmod) : z - Zmod;

		FVector DropLocation = FVector(
			Impact.X >= 0 ? xRes : -xRes,
			Impact.Y >= 0 ? yRes : -yRes,
			Impact.Z >= 0 ? zRes : -zRes
		);

		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, DropLocation.ToCompactString());

		if (ItemToPut != nullptr) {
			UWorld* const World = GetWorld();
			if (World != nullptr && ItemToPut->ToolType == 1) {
				bool bNoCollisionFail = true;
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				ActorSpawnParams.bNoFail = true;

				ABlock* Block = Cast<ABlock>(World->SpawnActor<AActor>(ItemToPut->BlockType, DropLocation, FRotator::ZeroRotator));

				if (PutSound != nullptr)
				{
					UGameplayStatics::PlaySoundAtLocation(this, PutSound, GetActorLocation());
				}

				//ItemToPut->Hide(true);
				ItemToPut->Destroy();

				Inventory[CurrentInventorySlot] = NULL;
				UpdateWieldedItem();
			}
		}
	}
}

void AUltraMinecraftCharacter::ShowHideCrafting()
{
	AUltraMinecraftGameMode * mymode = Cast<AUltraMinecraftGameMode>(GetWorld()->GetAuthGameMode());
	if (mymode->GetHUDState() == mymode->EHUDState::HS_Craft_Menu) {
		mymode->ChangeHUDState(mymode->EHUDState::HS_Ingame);
	}
	else {
		mymode->ChangeHUDState(mymode->EHUDState::HS_Craft_Menu);
	}

}

void AUltraMinecraftCharacter::MoveDownInventorySlot()
{
	CurrentInventorySlot++;
	if (CurrentInventorySlot >= NUM_OF_INVENTORY_SLOTS) {
		CurrentInventorySlot = 0;
	}
	UpdateWieldedItem();
}

void AUltraMinecraftCharacter::MoveUpInventorySlot()
{
	CurrentInventorySlot--;
	if (CurrentInventorySlot < 0) {
		CurrentInventorySlot = NUM_OF_INVENTORY_SLOTS - 1;
	}
	UpdateWieldedItem();
}

void AUltraMinecraftCharacter::OnHit()
{
	PlayHitAnim();

	if (CurrentBlock != nullptr) {
		bIsBreaking = true;

		int value = 2; // hardcored
		//int value = CurrentTool->Power;

		float Resistance = 0.4f;
		//float Resistance = CurrentBlock->Resistance;

		float TimeBeetweenBreaks = ((CurrentBlock->Resistance) / 100.f) / value;

		GetWorld()->GetTimerManager().SetTimer(BlockBreakingHandler, this, &AUltraMinecraftCharacter::BreakBlock, TimeBeetweenBreaks, true);
		GetWorld()->GetTimerManager().SetTimer(HitAnimHandle, this, &AUltraMinecraftCharacter::PlayHitAnim, Resistance, true);
	}
}

void AUltraMinecraftCharacter::EndHit()
{
	GetWorld()->GetTimerManager().ClearTimer(BlockBreakingHandler);
	GetWorld()->GetTimerManager().ClearTimer(HitAnimHandle);

	bIsBreaking = false;

	if (CurrentBlock != nullptr) {
		CurrentBlock->ResetBlock();
	}
}

void AUltraMinecraftCharacter::PlayHitAnim()
{
	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AUltraMinecraftCharacter::CheckForBlocks()
{
	FHitResult LinetracehHit;

	FVector StartTrace = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndTrace = (FirstPersonCameraComponent->GetForwardVector() * Reach) + StartTrace;

	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(LinetracehHit, StartTrace, EndTrace, ECollisionChannel::ECC_WorldDynamic, CQP);

	ABlock* PotentialBlock = Cast<ABlock>(LinetracehHit.GetActor());

	if (CurrentBlock != nullptr && PotentialBlock != CurrentBlock) {
		CurrentBlock->ResetBlock();
	}

	if (PotentialBlock == NULL) {
		CurrentBlock = nullptr;
		return;
	}
	else {
		if (CurrentBlock != nullptr && !bIsBreaking) {
			CurrentBlock->ResetBlock();
		}

		CurrentBlock = PotentialBlock;
		//debug
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, *CurrentBlock->GetName());
	}
}

void AUltraMinecraftCharacter::BreakBlock()
{
	if (bIsBreaking && CurrentBlock != nullptr && !CurrentBlock ->IsPendingKill()) {
		CurrentBlock->Break();
	}
}