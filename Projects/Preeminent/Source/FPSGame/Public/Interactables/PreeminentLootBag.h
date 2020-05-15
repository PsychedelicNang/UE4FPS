// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreeminentLootBag.generated.h"

/**
 * Loot bag used for transporting money in Preeminent
 */
UCLASS()
class PREEMINENT_API APreeminentLootBag : public AActor
{
	GENERATED_BODY()

private:
	/* Is this Loog Bag being equipped to a pawn?*/
	bool bPendingEquip;

	/* Is this Loot Bag equipped to a pawn?*/
	bool bIsEquipped;

	/* Timer used for dropping a loot bag after it has been told to stop moving*/
	FTimerHandle TimerHandle_WaitToDrop;

protected:
	/** Default amount of money stored in this loot bag*/
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	int32 DefaultMoneyStored;

	/** Current amount of money stored in this loot bag*/
	int32 CurrentMoneyStored;

	/** Movement speed modifier for when an actor picks up a loot bag*/
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	float CarryingSpeedModifier;

	/** Owning pawn */
	UPROPERTY(Transient)
	class APreeminentCharacter* MyPawn;

	/** Mesh used for a LootBag*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties)
	class UStaticMeshComponent* MeshComp;

	/** Primitive component for handling physics*/
	class UPrimitiveComponent* PrimComp;

public:	
	/** Sets default values for this actor's properties*/
	APreeminentLootBag();

protected:
	/** Called when the game starts or when spawned*/
	virtual void BeginPlay() override;

	//////////////////////////////////////////////////////////////////////////
	// LootBag movement

	void FinishDropping();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** attaches mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches mesh from pawn */
	void DetachMeshFromPawn();

public:	

	bool IsAttachedToPawn() const;

	// Converts the mass of this loot bag from grams (mass of $1) to Kg
	float GetMassOfBagInKg() const;

	FVector GetMeshExtents() const;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** set the LootBag's owning pawn */
	void SetOwningPawn(class APreeminentCharacter* APreeminentCharacter);

	/** LootBag is being equipped by owner pawn */
	virtual void OnEquip();

	/** LootBag is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** LootBag has been dropped or thrown by owner pawn */
	virtual void OnUnEquip();

	/** [server] LootBag was added to pawn's inventory */
	virtual void OnEnterInventory(class APreeminentCharacter* NewOwner);

	/** [server] LootBag was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	//////////////////////////////////////////////////////////////////////////
	// LootBag movement

	void LaunchItem(FVector LaunchVelocity);

	void StopMovementAndDrop();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** equip LootBag */
	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipLootBag();

	//////////////////////////////////////////////////////////////////////////
	// LootBag movement

	/** equip LootBag */
	UFUNCTION(reliable, server, WithValidation)
		void ServerLaunchItem(FVector LaunchVelocity);

	//////////////////////////////////////////////////////////////////////////
	// Accessors

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetCarryingLootBagSpeedModifier() const;
};
