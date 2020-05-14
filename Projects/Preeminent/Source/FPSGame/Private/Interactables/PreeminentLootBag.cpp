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

// Sets default values
ALootBag::ALootBag()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	//MeshComp->SetSimulatePhysics(true);
	//MeshComp->GetPhysicsComponent
	//MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	//UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetComponentByClass(UPrimitiveComponent::StaticClass()));
	//PrimComp->SetMassOverrideInKg(NAME_None, 10.0f, true);
	//SphereCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollisionComponent"));
	//SphereCollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//SphereCollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // We only care about collision with Pawns
	//SphereCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ALootBag::OnBeginOverlapSphereCollisionComponent);
	//SphereCollisionComp->OnComponentEndOverlap.AddDynamic(this, &ALootBag::OnEndOverlapSphereCollisionComponent);
	//SphereCollisionComp->SetupAttachment(MeshComp);
	//SphereCollisionComp->SetSphereRadius(125.0f);

	DefaultMoneyStored = 50000;
	CarryingSpeedModifier = 0.5f;
	CurrentMoneyStored = DefaultMoneyStored;

	PrimComp = Cast<UPrimitiveComponent>(GetComponentByClass(UPrimitiveComponent::StaticClass()));
	PrimComp->SetSimulatePhysics(true);
	PrimComp->SetMassOverrideInKg(NAME_None, GetMassOfBagInKg(), true);

	bReplicateMovement = true;
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALootBag::BeginPlay()
{
	Super::BeginPlay();

	CurrentMoneyStored = DefaultMoneyStored;
	CarryingSpeedModifier =  1 - (CurrentMoneyStored * 0.00001f);
	PrimComp->SetMassOverrideInKg(NAME_None, GetMassOfBagInKg(), true);
}

void ALootBag::FinishDropping()
{
	// Set our linear damping back to 0 so gravity affects us and we land on the ground
	PrimComp->SetLinearDamping(0.0f);
}

//void ALootBag::OnBeginOverlapSphereCollisionComponent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
//{
//	UE_LOG(LogTemp, Log, TEXT("Overlapped!"));
//	
//	// Only allow someone to pick us up if we're not already owned
//	if (OtherActor && !IsAttachedToPawn())
//	{
//		AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);
//	}
//}
//
//void ALootBag::OnEndOverlapSphereCollisionComponent(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
//{
//	UE_LOG(LogTemp, Log, TEXT("Stopped Overlapping!"));
//
//	// Stop the actor from being able to pick us up
//	if (OtherActor)
//	{
//		AShooterCharacter* Player = Cast<AShooterCharacter>(OtherActor);
//	}
//}

// Called every frame
void ALootBag::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (bIsEquipped)
		//UE_LOG(LogTemp, Log, TEXT("%s %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"), (PrimComp->IsCollisionEnabled()) ? TEXT("True") : TEXT("False"));
}

float ALootBag::GetCarryingLootBagSpeedModifier() const
{
	return CarryingSpeedModifier;
}

void ALootBag::DetachMeshFromPawn()
{
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	PrimComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ALootBag::OnEnterInventory(AShooterCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void ALootBag::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}

bool ALootBag::IsAttachedToPawn() const
{
	return  bIsEquipped || bPendingEquip;
	//return bIsEquipped || bPendingEquip;
}
float ALootBag::GetMassOfBagInKg() const
{
	return CurrentMoneyStored * 0.001f;
}

void ALootBag::LaunchItem(FVector LaunchVelocity)
{
	if (PrimComp)
	{
		PrimComp->AddImpulse(LaunchVelocity, NAME_None, true);
	}
	//if (Role == ROLE_Authority)
	//{
	//	if (PrimComp)
	//	{
	//		PrimComp->AddImpulse(LaunchVelocity, NAME_None, true);
	//	}
	//}
	//else
	//{
	//	ServerLaunchItem(LaunchVelocity);
	//}
}

bool ALootBag::ServerLaunchItem_Validate(FVector LaunchVelocity)
{
	return true;
}

void ALootBag::ServerLaunchItem_Implementation(FVector LaunchVelocity)
{
	LaunchItem(LaunchVelocity);
}

void ALootBag::StopMovementAndDrop()
{
	// Set our linear damping so we ignore the current impulse being applied to the player
	PrimComp->SetLinearDamping(FLT_MAX);

	// Wait half a second before we drop the bag
	GetWorldTimerManager().SetTimer(TimerHandle_WaitToDrop, this, &ALootBag::FinishDropping, 0.5f, false, 0.0f);

	// Note: We cannot stop physics or change the collision properties because it will say we stopped colliding with the extraction zone (Say we are no longer in the zone when in fact we are)
}

FVector ALootBag::GetMeshExtents() const
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

void ALootBag::OnUnEquip()
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

void ALootBag::SetOwningPawn(AShooterCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}


//////////////////////////////////////////////////////////////////////////
// Inventory

bool ALootBag::ServerEquipLootBag_Validate()
{
	return true;
}

void ALootBag::ServerEquipLootBag_Implementation()
{
	OnEquip();
}

void ALootBag::OnEquip()
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

	//	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &ALootBag::OnEquipFinished, Duration, false);
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

void ALootBag::OnEquipFinished()
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

void ALootBag::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetLootBagAttachPoint();

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

