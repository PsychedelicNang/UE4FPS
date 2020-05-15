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

	/*Use GENERATED_BODY() if using a default constructor*/
	//	GENERATED_BODY()
	//public:
	//	APreeminentCharacter();

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

	/* True if this pawn is requesting to pick up a LootBag from a stockpile*/
	bool bPendingRequestLootBag;

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

	bool bWantsToZoom;

	// Pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		uint8 bDied : 1;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 45, ClampMax = 120.0f))
		float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0f))
		float ZoomedInterpSpeed;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn, meta = (ClampMin = 0.1, ClampMax = 1.0f))
		float TargetingSpeedModifier;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn, meta = (ClampMin = 1.0, ClampMax = 10.0f))
		float RunningSpeedModifier;

	UPROPERTY(Transient, Replicated)
		uint8 bWantsToRun : 1;

	bool bWantsToRunToggled;

	UPROPERTY(Transient, Replicated)
		uint8 bIsTargeting : 1;

	uint8 bWantsToFire;

	UPROPERTY(Transient, Replicated)
		uint8 bIsCarryingLootBag : 1;

	bool bCanInteractWithObj;

	// Distance from which this pawn can interact with
	UPROPERTY(EditDefaultsOnly, Category = Player)
		float InteractableDistance;

	float LaunchStrength;

	uint8 bIsAttemptingInteract : 1;
	float InteractHeldTime;

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

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_CurrentLootBag)
	class APreeminentLootBag* CurrentLootBag;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory, ReplicatedUsing = OnRep_CurrentWeapon)
	class APreeminentWeapon* CurrentWeapon;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//////////////////////////////////////////////////////////////////////////
	// Event handlers

	UFUNCTION()
		void OnHealthChanged(class UPreeminentHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void StartJump();

	void StopJump();

	void BeginZoom();

	void EndZoom();

	void Reload();

	void StartInteract();

	void EndInteract();

	/** player pressed run action */
	void OnStartRunning();

	/** player pressed toggled run action */
	void OnStartRunningToggle();

	/** player released run action */
	void OnStopRunning();

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	/** [local] starts weapon fire */
	void OnStartFiring();

	/** [local] stops weapon fire */
	void OnStopFiring();

	void OnThrowPressed();

	/** player pressed next weapon action */
	void OnNextWeapon();

	/** player pressed prev weapon action */
	void OnPrevWeapon();

	//////////////////////////////////////////////////////////////////////////
	// Action handling

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	///** [server + local] change targeting state */
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

	void OnThrowItem(FVector CamLoc, FRotator CamRot);

	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFiring();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFiring();

	void GetLootBagFromStockpile(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester);

	/*Should only be used in development.*/
	UFUNCTION(BlueprintCallable)
		bool RequestRestartDeadPlayers();

	UFUNCTION(reliable, server, WithValidation)
		void ServerRequestRestartDeadPlayers();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** updates current weapon */
	void SetCurrentWeapon(class APreeminentWeapon* NewWeapon, class APreeminentWeapon* LastWeapon = nullptr);

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

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipLootBag(class APreeminentLootBag* LootBag);

	/** current weapon rep handler */
	UFUNCTION()
		void OnRep_CurrentLootBag(class APreeminentLootBag* LootBag);

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
		void ServerThrowItem(FVector CamLoc, FRotator CamRot);

	UFUNCTION(NetMulticast, reliable)
		void MulticastOnThrowItem(FVector CamLoc, FRotator CamRot);

	UFUNCTION(reliable, server, WithValidation)
		void ServerRequestLootBag(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester);

public:
	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/*
	* Get either first or third person mesh.
	*
	* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
	*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;

	//////////////////////////////////////////////////////////////////////////
	// Accessor methods

	virtual FVector GetPawnViewLocation() const override;

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		FRotator GetAimOffsets() const;

	/** get camera view type */
	UFUNCTION(BlueprintCallable, Category = Mesh)
		virtual bool IsFirstPerson() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		class APreeminentWeapon* GetWeapon() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsRunning() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		bool IsTargeting() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		bool IsFiring() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		bool IsCarryingLootBag() const;

	/** get weapon taget modifier speed	*/
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
		float GetTargetingSpeedModifier() const;

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetRunningSpeedModifier() const;

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetCarryingLootBagSpeedModifier() const;

	/** get weapon attach point */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		FName GetWeaponAttachPointName() const;

	/** get weapon attach point */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		FName GetLootBagAttachPointName() const;
};
