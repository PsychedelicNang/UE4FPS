// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreeminentLootBag.generated.h"

class APreeminentCharacter;
class UPrimitiveComponent;

UCLASS()
class PREEMINENT_API APreeminentLootBag : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = Properties)
	// Default amount of money stored in this loot bag
	int32 DefaultMoneyStored;

	// Current amount of money stored in this loot bag
	int32 CurrentMoneyStored;

	/** Pawn mesh: 1st person view  */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Properties)
		class UStaticMeshComponent* MeshComp;

	/** modifier for when an actor picks up this loot bag  */
	UPROPERTY(EditDefaultsOnly, Category = Properties)
		float CarryingSpeedModifier;

	///** socket or bone name for attaching weapon mesh */
	//UPROPERTY(EditDefaultsOnly, Category = Properties)
	//	FName WeaponAttachPoint;

	//UPROPERTY(VisibleAnywhere, Category = "Components")
	//	// Detects if we are colliding with something that can pick us up
	//	class USphereComponent* SphereCollisionComp;

	bool bPendingEquip;
	bool bIsEquipped;

	/** pawn owner */
	UPROPERTY(Transient)
		APreeminentCharacter* MyPawn;

	UPrimitiveComponent* PrimComp;

	bool bBeginDrop;

	FTimerHandle TimerHandle_WaitToDrop;

public:	
	// Sets default values for this actor's properties
	APreeminentLootBag();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void FinishDropping();

	//UFUNCTION()
	//	void OnBeginOverlapSphereCollisionComponent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//
	//UFUNCTION()
	//	void OnEndOverlapSphereCollisionComponent(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//////////////////////////////////////////////////////////////////////////
// Inventory

/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
		float GetCarryingLootBagSpeedModifier() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(APreeminentCharacter* APreeminentCharacter);

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(APreeminentCharacter* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	bool IsAttachedToPawn() const;

	// Converts the mass of this loot bag from grams (mass of $1) to Kg
	float GetMassOfBagInKg() const;

	void LaunchItem(FVector LaunchVelocity);

	void StopMovementAndDrop();

	FVector GetMeshExtents() const;

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
		void ServerEquipLootBag();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
		void ServerLaunchItem(FVector LaunchVelocity);

};
