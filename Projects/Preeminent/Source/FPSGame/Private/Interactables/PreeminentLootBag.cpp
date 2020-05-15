// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentLootBag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player/PreeminentCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Interactables/PreeminentLootBag.h"

APreeminentLootBag::APreeminentLootBag()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComp;

	DefaultMoneyStored = 50000;
	CarryingSpeedModifier = 0.5f;
	CurrentMoneyStored = DefaultMoneyStored;

	PrimComp = Cast<UPrimitiveComponent>(GetComponentByClass(UPrimitiveComponent::StaticClass()));
	PrimComp->SetSimulatePhysics(true);
	PrimComp->SetMassOverrideInKg(NAME_None, GetMassOfBagInKg(), true);

	bReplicateMovement = true;
	SetReplicates(true);
}

void APreeminentLootBag::BeginPlay()
{
	Super::BeginPlay();

	CurrentMoneyStored = DefaultMoneyStored;
	CarryingSpeedModifier =  1 - (CurrentMoneyStored * 0.00001f);
	PrimComp->SetMassOverrideInKg(NAME_None, GetMassOfBagInKg(), true);
}

//////////////////////////////////////////////////////////////////////////
// LootBag movement

void APreeminentLootBag::LaunchItem(FVector LaunchVelocity)
{
	if (PrimComp)
	{
		PrimComp->AddImpulse(LaunchVelocity, NAME_None, true);
	}
}

bool APreeminentLootBag::ServerLaunchItem_Validate(FVector LaunchVelocity)
{
	return true;
}

void APreeminentLootBag::ServerLaunchItem_Implementation(FVector LaunchVelocity)
{
	LaunchItem(LaunchVelocity);
}

void APreeminentLootBag::StopMovementAndDrop()
{
	// Set our linear damping so we ignore the current impulse being applied to the player
	PrimComp->SetLinearDamping(FLT_MAX);

	// Wait half a second before we drop the bag
	GetWorldTimerManager().SetTimer(TimerHandle_WaitToDrop, this, &APreeminentLootBag::FinishDropping, 0.5f, false, 0.0f);

	// Note: We cannot stop physics or change the collision properties because it will say we stopped colliding with the extraction zone (Say we are no longer in the zone when in fact we are)
}

void APreeminentLootBag::FinishDropping()
{
	// Set our linear damping back to 0 so gravity affects us and we land on the ground
	PrimComp->SetLinearDamping(0.0f);
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void APreeminentLootBag::SetOwningPawn(APreeminentCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

void APreeminentLootBag::DetachMeshFromPawn()
{
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	PrimComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APreeminentLootBag::OnEnterInventory(APreeminentCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void APreeminentLootBag::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(nullptr);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

void APreeminentLootBag::OnUnEquip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;

	//StopFire();

	//if (bPendingReload)
	//{
	//	StopWeaponAnimation(ReloadAnim);
	//	bPendingReload = false;

	//	GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
	//	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	//}

	if (bPendingEquip)
	{
		//StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		//GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	//DetermineWeaponState();
}

bool APreeminentLootBag::ServerEquipLootBag_Validate()
{
	return true;
}

void APreeminentLootBag::ServerEquipLootBag_Implementation()
{
	OnEquip();
}

void APreeminentLootBag::OnEquip()
{
	AttachMeshToPawn();
	bPendingEquip = true;
	OnEquipFinished();

	//if (Role == ROLE_Authority)
	//{
	//	AttachMeshToPawn();
	//	bPendingEquip = true;
	//	OnEquipFinished();
	//}
	//else
	//{
	//	ServerEquipLootBag();
	//	PrimComp->SetSimulatePhysics(false);
	//}


	//AttachMeshToPawn();
	//bPendingEquip = true;
	//OnEquipFinished();

	//DetermineWeaponState();

	//// Only play animation if last weapon is valid
	//if (LastWeapon)
	//{
	//	float Duration = PlayWeaponAnimation(EquipAnim);
	//	if (Duration <= 0.0f)
	//	{
	//		// failsafe
	//		Duration = 0.5f;
	//	}
	//	EquipStartedTime = GetWorld()->GetTimeSeconds();
	//	EquipDuration = Duration;

	//	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &APreeminentLootBag::OnEquipFinished, Duration, false);
	//}
	//else
	//{
	//	OnEquipFinished();
	//}
	//OnEquipFinished();

	//if (MyPawn && MyPawn->IsLocallyControlled())
	//{
	//	PlayWeaponSound(EquipSound);
	//}
}

void APreeminentLootBag::OnEquipFinished()
{
	//AttachMeshToPawn();

	bIsEquipped = true;
	bPendingEquip = false;

	//// Determine the state so that the can reload checks will work
	//DetermineWeaponState(); 
	//
	//if (MyPawn)
	//{
	//	// try to reload empty clip
	//	if (MyPawn->IsLocallyControlled() &&
	//		CurrentAmmoInClip <= 0 &&
	//		CanReload())
	//	{
	//		StartReload();
	//	}
	//}


}

void APreeminentLootBag::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetLootBagAttachPointName();

		//UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *AttachPoint.ToString());

		PrimComp->SetSimulatePhysics(false);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
			MeshComp->SetHiddenInGame(false);
			MeshComp->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::SnapToTargetIncludingScale,  AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			MeshComp->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
			MeshComp->SetHiddenInGame(false);
		}

		UE_LOG(LogTemp, Log, TEXT("Grabbed loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));

		//DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 12, FColor::Green, false, 5.0f, 0, 1.0f);
		//DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(124, -50, 121), 10.0f, 12, FColor::Green, false, 5.0f, 0, 1.0f);
	}
}

//////////////////////////////////////////////////////////////////////////
// Accessors & Public methods

float APreeminentLootBag::GetCarryingLootBagSpeedModifier() const
{
	return CarryingSpeedModifier;
}

bool APreeminentLootBag::IsAttachedToPawn() const
{
	return  bIsEquipped || bPendingEquip;
}

float APreeminentLootBag::GetMassOfBagInKg() const
{
	return CurrentMoneyStored * 0.001f;
}

FVector APreeminentLootBag::GetMeshExtents() const
{
	if (MeshComp)
	{
		FVector Min;
		FVector Max;
		MeshComp->GetLocalBounds(Min, Max);
		return Max - Min;
	}
	else
	{
		return FVector::ZeroVector;
	}
}