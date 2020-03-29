// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "FPSGame.h"
#include "Net/UnrealNetwork.h"
#include "ShooterCharacter.h"
#include "Particles/ParticleSystemComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("Shooter.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

// Sets default values
AShooterWeapon::AShooterWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleFlashSocket";
	TracerTargetName = "BeamEnd";

	MagazineCapacity = WeaponConfig.AmmoPerClip;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	MagazineSize = MagazineCapacity;

	MyPawn = Cast<AShooterCharacter>(GetOwner());
}

// Called every frame
void AShooterWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

EWeaponState::Type AShooterWeapon::GetCurrentState() const
{
	return CurrentState;
}

void AShooterWeapon::OnEnterInventory(AShooterCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AShooterWeapon::OnLeaveInventory()
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

bool AShooterWeapon::IsAttachedToPawn() const
{
	return  bIsEquipped || bPendingEquip;
	//return bIsEquipped || bPendingEquip;
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AShooterWeapon::OnEquip(const AShooterWeapon* LastWeapon)
{
	AttachMeshToPawn();

	bPendingEquip = true;
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

	//	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AShooterWeapon::OnEquipFinished, Duration, false);
	//}
	//else
	//{
	//	OnEquipFinished();
	//}
	OnEquipFinished();

	//if (MyPawn && MyPawn->IsLocallyControlled())
	//{
	//	PlayWeaponSound(EquipSound);
	//}
}

void AShooterWeapon::OnEquipFinished()
{
	AttachMeshToPawn();

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

void AShooterWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();

		//UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *AttachPoint.ToString());

		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
			//USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecifcPawnMesh(false);
			MeshComp->SetHiddenInGame(false);
			//Mesh3P->SetHiddenInGame(false);
			bool result = MeshComp->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			//Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			
			if (result)
			{
				UE_LOG(LogTemp, Log, TEXT("Attached"));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Not"));
			}
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			bool result = UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);

			if (result)
			{
				UE_LOG(LogTemp, Log, TEXT("Attached (else)"));
				UE_LOG(LogTemp, Log, TEXT("test!"));
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Not (else)"));
			}
		}
	}
}

USkeletalMeshComponent* AShooterWeapon::GetWeaponMesh() const
{
	return MeshComp;
	//return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

void AShooterWeapon::DetachMeshFromPawn()
{
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	MeshComp->SetHiddenInGame(true);
	UE_LOG(LogTemp, Log, TEXT("Set Hidden %s"), *MeshComp->GetName());

	//Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	//Mesh3P->SetHiddenInGame(true);
}

void AShooterWeapon::OnUnEquip()
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

void AShooterWeapon::SetOwningPawn(AShooterCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

float AShooterWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation.Pawn1P;
		//UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AShooterWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = Animation.Pawn1P;

		//UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

void AShooterWeapon::Fire()
{
	if (MagazineSize > 0 || WeaponConfig.bInfiniteClip)
	{
		FireWeapon();
		MagazineSize--;
	}
	else
	{
		StopFiring();
	}
}

bool AShooterWeapon::ServerStartFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStartFire_Implementation()
{
	BeginFiring();
}

bool AShooterWeapon::ServerStopFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStopFire_Implementation()
{
	StopFiring();
}

void AShooterWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

void AShooterWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool AShooterWeapon::ServerStartReload_Validate()
{
	return true;
}

bool AShooterWeapon::ServerStopReload_Validate()
{
	return true;
}

void AShooterWeapon::BeginFiring()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}
	
	float FirstDelay = FMath::Max(LastFiredTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AShooterWeapon::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);
}

void AShooterWeapon::StopFiring()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	StopWeaponAnimation(FireAnim);

	//if (MuzzlePSC)
	//{
	//	MuzzlePSC->DeactivateSystem();
	//	MuzzlePSC = nullptr;
	//}
}

void AShooterWeapon::StartReload()
{
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	MagazineSize = MagazineCapacity;
}

void AShooterWeapon::StopReload()
{
	// TODO: Implement...
}

void AShooterWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void AShooterWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void AShooterWeapon::ServerFire_Implementation()
{
	Fire();
}

bool AShooterWeapon::ServerFire_Validate()
{
	// This is where anti-cheat should happen.
	return true;
}

void AShooterWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX

	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}


void AShooterWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterWeapon, HitScanTrace, COND_SkipOwner);
}
