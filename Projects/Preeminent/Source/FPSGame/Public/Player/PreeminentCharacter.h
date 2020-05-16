// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PreeminentCharacter.generated.h"

/*
* Main character (Pawn) in Preeminent
*/
UCLASS(Abstract)
class PREEMINENT_API APreeminentCharacter : public ACharacter
{
	/*Use GENERATED_UCLASS_BODY() if using a member initializer list*/
	GENERATED_UCLASS_BODY()

	//////////////////////////////////////////////////////////////////////////
	// BEGIN: UNUSED BUT KEEP

	/*Use GENERATED_BODY() if using a default constructor*/
	//	GENERATED_BODY()
	//public:
	//	APreeminentCharacter();
	
	// END: UNUSED BUT KEEP
	//////////////////////////////////////////////////////////////////////////

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

	//////////////////////////////////////////////////////////////////////////
	// Fields

	/* True if this pawn is requesting to pick up a LootBag from a stockpile*/
	bool bPendingRequestLootBag;

	/* True if the pawn wants to target*/
	bool bWantsToZoom;
	
	/* Default Field of View value for this pawn. Will be used if not targeting or zoomed*/
	float DefaultFOV;

	/* Amount of time the interact button needs to be held in order to initiate the interact mechanic*/
	float InteractHeldTime;

	/* True if the pawn is attempting to interact */
	uint8 bIsAttemptingInteract : 1;

	/* True if this pawn wants to run*/
	UPROPERTY(Transient, Replicated)
		uint8 bWantsToRun : 1;

	/* True if this pawn wants to run and is toggled*/
	bool bWantsToRunToggled;

	/* True if this pawn wants to target*/
	UPROPERTY(Transient, Replicated)
		uint8 bIsTargeting : 1;

	/* True if this pawn wants to fire their weapon*/
	uint8 bWantsToFire;

	/* True if this pawn is carrying a loot bag*/
	UPROPERTY(Transient, Replicated)
		uint8 bIsCarryingLootBag : 1;

	// True if pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		uint8 bDied : 1;

protected:

	//////////////////////////////////////////////////////////////////////////
	// Components

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
		USkeletalMeshComponent* Mesh1PComp;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPreeminentHealthComponent* HealthComp;

	//////////////////////////////////////////////////////////////////////////
	// Fields

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 45, ClampMax = 120.0f))
		float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0f))
		float ZoomedInterpSpeed;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Player, meta = (ClampMin = 0.1, ClampMax = 1.0f))
		float TargetingSpeedModifier;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Player, meta = (ClampMin = 1.0, ClampMax = 10.0f))
		float RunningSpeedModifier;

	/* Distance in UE4 units this pawn can interact with interactable items*/
	UPROPERTY(EditDefaultsOnly, Category = Player, meta = (ClampMin = 1.0, ClampMax = 500.0f))
	float InteractableDistance;

	/* Distance in UE4 units this pawn can throw an item */
	UPROPERTY(EditDefaultsOnly, Category = Player, meta = (ClampMin = 100.0, ClampMax = 10000.0f))
	float LaunchStrength;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		FName WeaponAttachPointName;

	/** socket or bone name for attaching lootbags */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		FName LootBagAttachPointName;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		TArray<TSubclassOf<class APreeminentWeapon> > DefaultInventoryClasses;

	/** weapons in inventory */
	UPROPERTY(Transient, Replicated)
		TArray<class APreeminentWeapon*> Inventory;

	/* Current loot bag being carried by this pawn. Can be nullptr */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CurrentLootBag)
	class APreeminentLootBag* CurrentLootBag;

	/* Current weapon being handled by this pawn. Can be nullptr*/
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory, ReplicatedUsing = OnRep_CurrentWeapon)
	class APreeminentWeapon* CurrentWeapon;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	//////////////////////////////////////////////////////////////////////////
	// Event handlers

	UFUNCTION()
		void OnHealthChanged(class UPreeminentHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	// [local] Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** [local] player pressed move forward action */
	void MoveForward(float Value);

	/** [local] player pressed move right action */
	void MoveRight(float Value);

	/** [local] begins crouching mechanic */
	void BeginCrouch();

	/** [local] ends crouching mechanic */
	void EndCrouch();

	/** [local] starts jumping mechanic */
	void StartJump();

	/** [local] stops jumping mechanic */
	void StopJump();

	/** [local] starts zooming mechanic (player started targeting) */
	void BeginZoom();

	/** [local] stops zooming mechanic (player stopped targeting) */
	void EndZoom();

	/** [local] player pressed reload action */
	void Reload();

	/** [local] start interact mechanic */
	void StartInteract();

	/** [local] end interact mechanic */
	void EndInteract();

	/** [local] starts running mechanic */
	void OnStartRunning();

	/** [local] starts toggled running mechanic */
	void OnStartRunningToggle();

	/** [local] stops running mechanic */
	void OnStopRunning();

	/** [local] starts targeting mechanic */
	void OnStartTargeting();

	/** [local] stops targeting mechanic */
	void OnStopTargeting();

	/** [local] starts firing weapon */
	void OnStartFiring();

	/** [local] stops firing weapon */
	void OnStopFiring();

	/* [local] starts throwing mechanic*/
	void OnThrowPressed();

	/** [local] player pressed next weapon action */
	void OnNextWeapon();

	/** [local] player pressed prev weapon action */
	void OnPrevWeapon();

	//////////////////////////////////////////////////////////////////////////
	// Action handling

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	/** [server + local] change targeting state */
	void SetTargeting(bool bNewTargeting);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
		void ServerSetTargeting(bool bNewTargeting);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
		void ServerSetRunning(bool bNewRunning, bool bToggle);

	/*Responds to a weapon being fired from this character*/
	UFUNCTION()
		void HandleOnWeaponFired(class APreeminentWeapon* WeaponFired);

	/** [server + local] start throw mechanic */
	void OnThrowItem(FVector CamLoc, FRotator CamRot);

	/** [local] begin firing weapon */
	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFiring();

	/** [local] stop firing weapon */
	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFiring();

	/** [server + local] Get a loot bag from the stockpile
	* @param Stockpile Stockpile to get loot bag from
	* @param Requester Character requesting the loot bag
	*/
	void GetLootBagFromStockpile(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester);

	/** request loot bag from stockpile */
	UFUNCTION(reliable, server, WithValidation)
	void ServerRequestLootBag(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester);

	/*request to restart dead players
	* @warning Should only be used in development!
	*/
	UFUNCTION(BlueprintCallable)
		bool RequestRestartDeadPlayers();

	/* request to restart dead players*/
	UFUNCTION(reliable, server, WithValidation)
		void ServerRequestRestartDeadPlayers();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** [local] updates current weapon */
	void SetCurrentWeapon(class APreeminentWeapon* NewWeapon, class APreeminentWeapon* LastWeapon = nullptr);

	/** [local] updates current loot bag */
	void SetCurrentLootBag(APreeminentLootBag* LootBag);

	/**
	* [server] add weapon to inventory
	* @param Weapon	Weapon to add.
	*/
	void AddWeapon(class APreeminentWeapon* Weapon);

	/**
	* [server] remove weapon from inventory
	* @param Weapon	Weapon to remove.
	*/
	void RemoveWeapon(class APreeminentWeapon* Weapon);

	/**
	* Find in inventory
	* @param WeaponClass	Class of weapon to find.
	*/
	class APreeminentWeapon* FindWeapon(TSubclassOf<class APreeminentWeapon> WeaponClass);

	/**
	* [server + local] equips weapon from inventory
	* @param Weapon	Weapon to equip
	*/
	void EquipWeapon(class APreeminentWeapon* Weapon);

public:
	/**
	* [server + local] equips loot bag from inventory
	* @param LootBag loot bag to equip
	*/
	void EquipLootBag(class APreeminentLootBag* LootBag);

public:

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipWeapon(class APreeminentWeapon* NewWeapon);

	/** current weapon rep handler */
	UFUNCTION()
		void OnRep_CurrentWeapon(class APreeminentWeapon* LastWeapon);

	/** equip loot bag */
	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipLootBag(class APreeminentLootBag* LootBag);

	/** current loot bag rep handler */
	UFUNCTION()
		void OnRep_CurrentLootBag(class APreeminentLootBag* LootBag);

	/** throw item*/
	UFUNCTION(reliable, server, WithValidation)
		void ServerThrowItem(FVector CamLoc, FRotator CamRot);

	/* throw item*/
	UFUNCTION(NetMulticast, reliable)
		void MulticastOnThrowItem(FVector CamLoc, FRotator CamRot);

public:

	/*
	* Get either first or third person mesh.
	*
	* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
	*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;

	//////////////////////////////////////////////////////////////////////////
	// Animation handlers

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	//////////////////////////////////////////////////////////////////////////
	// Accessor methods

	/* get the location of the "eyes" (camera location) of the player*/
	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		FRotator GetAimOffsets() const;

	/** get camera view type */
	UFUNCTION(BlueprintCallable, Category = Mesh)
		virtual bool IsFirstPerson() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		class APreeminentWeapon* GetWeapon() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		bool IsTargeting() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsCarryingLootBag() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		float GetTargetingSpeedModifier() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetRunningSpeedModifier() const;

	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetCarryingLootBagSpeedModifier() const;

	/** Socket name where the weapon should attach to */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		FName GetWeaponAttachPointName() const;

	/** Socket name where the loot bag should attach to */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		FName GetLootBagAttachPointName() const;
};
