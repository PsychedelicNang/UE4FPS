// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS(Abstract)
class FPSGAME_API AShooterCharacter : public ACharacter
{
	/*Used GENERATED_UCLASS_BODY() if using a member initializer list*/
	GENERATED_UCLASS_BODY()

		/*Used GENERATED_BODY() if using a default constructor*/
	//	GENERATED_BODY()
	//
	//public:
	//	AShooterCharacter();

	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

public:
	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

protected:

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
		USkeletalMeshComponent* Mesh1PComp;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UShooterHealthComponent* HealthComp;

	bool bWantsToZoom;

	// Pawn died previously
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		bool bDied;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
		float ZoomedFOV;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100.0f))
		float ZoomedInterpSpeed;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
	class AShooterWeapon* CurrentWeapon;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		float TargetingSpeedModifier;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
		float RunningSpeedModifier;

	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		FName WeaponAttachPoint;

	/** socket or bone name for attaching lootbags */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		FName LootBagAttachPoint;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
		TArray<TSubclassOf<class AShooterWeapon> > DefaultInventoryClasses;

	/** weapons in inventory */
	UPROPERTY(Transient, Replicated)
		TArray<class AShooterWeapon*> Inventory;

	class ALootBag* CurrentLootBag;

	bool bWantsToRun;
	bool bWantsToRunToggled;
	bool bIsTargeting;
	bool bWantsToFire;

	bool bIsCarryingLootBag;

	bool bCanInteractWithObj;


	UPROPERTY(EditDefaultsOnly, Category = Player)
		// Distance from which this pawn can interact with distance from
		float InteractableDistance;

	float LaunchStrength;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void StartJump();

	void StopJump();

	void BeginZoom();

	void EndZoom();

	void Reload();

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	/** player pressed run action */
	void OnStartRunning();

	/** player pressed toggled run action */
	void OnStartRunningToggle();

	/** player released run action */
	void OnStopRunning();

	///** [server + local] change targeting state */
	void SetTargeting(bool bNewTargeting);

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	UFUNCTION()
		void OnHealthChanged(class UShooterHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/*Responds to a weapon being fired from this character*/
	UFUNCTION()
		void HandleOnWeaponFired(class AShooterWeapon* WeaponFired);

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();


	/** updates current weapon */
	void SetCurrentWeapon(class AShooterWeapon* NewWeapon, class AShooterWeapon* LastWeapon = nullptr);

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/**
	* [server] add weapon to inventory
	*
	* @param Weapon	Weapon to add.
	*/
	void AddWeapon(class AShooterWeapon* Weapon);

	/**
	* [server] remove weapon from inventory
	*
	* @param Weapon	Weapon to remove.
	*/
	void RemoveWeapon(class AShooterWeapon* Weapon);

	/**
	* Find in inventory
	*
	* @param WeaponClass	Class of weapon to find.
	*/
	class AShooterWeapon* FindWeapon(TSubclassOf<class AShooterWeapon> WeaponClass);

	/**
	* [server + local] equips weapon from inventory
	*
	* @param Weapon	Weapon to equip
	*/
	void EquipWeapon(class AShooterWeapon* Weapon);

	/** player pressed next weapon action */
	void OnNextWeapon();

	/** player pressed prev weapon action */
	void OnPrevWeapon();

	void OnThrowItem();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFiring();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFiring();

	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AShooterWeapon* GetWeapon() const;

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
	FName GetWeaponAttachPoint() const;

	/** get weapon attach point */
	FName GetLootBagAttachPoint() const;

	/*
* Get either first or third person mesh.
*
* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;
};
