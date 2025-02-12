// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Block.h"
#include "GameFramework/Character.h"
#include "Wieldable.h"
#include "UltraMinecraftCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AUltraMinecraftCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AUltraMinecraftCharacter();

	virtual void Tick(float DeltaTime) override;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* FP_ItemRight;

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AUltraMinecraftProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 InitialPlayerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 PlayerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	USoundCue* PutSound;

	/* Get current index */
	UFUNCTION(BlueprintPure, Category = HUD)
	int32 GetCurrentInventorySlot();

	/* Get current Item */
	UFUNCTION(BlueprintPure, Category = HUD)
	AWieldable* GetItemByIndex(int32 Index);

	/* Add inventory to our inventory */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool AddItemToInventory(AWieldable* Item);

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool HaveWieldableItemInInventory(TSubclassOf<class AWieldable> classType);

	/* Get Current inventory slot */
	UFUNCTION(BlueprintPure, Category = Inventory)
	UTexture2D* GetThumnailAtInventorySlot(int32 Slot);

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetNumStackAtInventorySlot(int32 Slot);

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool IsStackeableAtInventorySlot(int32 Slot);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool DecPlayerHealth(int32 Dec);

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetInitialHealth();

	UFUNCTION(BlueprintPure, Category = Inventory)
	int32 GetHealth();

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool IsHoldingWieldableItem(TSubclassOf<class AWieldable> classType);

	/* Add item to crafting */
	UFUNCTION(BlueprintCallable, Category = Crafting)
	bool AddItemToCraftingTableAt(int32 Index);

	UFUNCTION(BlueprintCallable, Category = Crafting)
	void GetCraftingItem();

	/* Get Thumnail crafting inventory slot */
	UFUNCTION(BlueprintPure, Category = Crafting)
	UTexture2D* GetThumnailAtCraftingInventorySlot(int32 Slot);

	/* Change Selected Inventory Slot  */
	UFUNCTION(BlueprintCallable, Category = HUDIngame)
	void MoveUpInventorySlot();

	UFUNCTION(BlueprintCallable, Category = HUDIngame)
	void MoveDownInventorySlot();

	uint8 ToolType;
	uint8 ToolMaterial;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

private:
	const int32 NUM_OF_INVENTORY_SLOTS = 9;
	const int32 NUM_OF_CRAFTING_INVENTORY_SLOTS = 10;	// 9 slots + 1 return (index = 9)

	/* Current inv slot */
	int32 CurrentInventorySlot;

	/* Update the wielded item */
	void UpdateWieldedItem();

	/* Gets the current wielded item */
	AWieldable* GetCurrentWieldedItem();

	/* Throw the current item*/
	void Throw();
	void Throw(AWieldable* ItemToThrow);

	/* Put the current item*/
	void Put();

	/* Show or hide Crafting HUD*/
	void ShowHideCrafting();

	/* Value of player is breaking a block*/
	bool bIsBreaking;

	/* Called when hitting with a tool */
	void OnHit();
	void EndHit();

	/* Plays tools animations */
	void PlayHitAnim();

	/* Check if there is a block in front to player */
	void CheckForBlocks();

	/* Called when we want to break a block */
	void BreakBlock();

	void AddItemToCraft(int32 Index);

	void UpdatePossiblyCraft();

	bool CheckCraftCorrect(int32 Index, FString& Item);

	/* Reference to block in front*/
	ABlock* CurrentBlock;

	/* The chracters reach (TODO: CHANGE TO PUBLIC) */
	float Reach;

	/* Timer handles */
	FTimerHandle BlockBreakingHandler;
	FTimerHandle HitAnimHandle;

	UPROPERTY(EditAnywhere)
	TArray<AWieldable*> Inventory;

	UPROPERTY(EditAnywhere)
	TArray<AWieldable*> CraftingInventory;

	UStaticMeshComponent* CopyMesh;

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

