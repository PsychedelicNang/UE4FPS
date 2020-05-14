// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentWeapon.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Preeminent.h"
#include "Net/UnrealNetwork.h"
#include "PreeminentCharacter.h"
#include "Particles/ParticleSystemComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("Shooter.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

// Sets default values
APreeminentWeapon::APreeminentWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	RootComponent = Mesh1P;

	MuzzleSocketName = "b_gun_muzzleflash";
	TracerTargetName = "BeamEnd";
	TracerStartName = "BulletTracerStart";

	MagazineCapacity = WeaponConfig.AmmoPerClip;
	
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	ADSAmount = 0.0;
}

void APreeminentWeapon::BeginPlay()
{
	Super::BeginPlay();

	MagazineSize = MagazineCapacity;

	MyPawn = Cast<APreeminentCharacter>(GetOwner());
}

// Called every frame
void APreeminentWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MyPawn)
	{
		if (MyPawn->IsTargeting())
		{
			ADSAmount = FMath::Clamp(ADSAmount + (1 / WeaponConfig.AimDownSightTime) * DeltaTime, 0.f, 1.f);
		}
		else
		{
			ADSAmount = FMath::Clamp(ADSAmount - (1 / WeaponConfig.AimDownSightTime) * DeltaTime, 0.f, 1.f);
		}
	}
}

EWeaponState::Type APreeminentWeapon::GetCurrentState() const
{
	return CurrentState;
}

void APreeminentWeapon::OnEnterInventory(APreeminentCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void APreeminentWeapon::OnLeaveInventory()
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

bool APreeminentWeapon::IsAttachedToPawn() const
{
	return  bIsEquipped || bPendingEquip;
	//return bIsEquipped || bPendingEquip;
}

float APreeminentWeapon::GetADSAmount() const
{
	return ADSAmount;
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void APreeminentWeapon::OnEquip(const APreeminentWeapon* LastWeapon)
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

	//	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &APreeminentWeapon::OnEquipFinished, Duration, false);
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

void APreeminentWeapon::OnEquipFinished()
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

void APreeminentWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();

		if (MyPawn->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecifcPawnMesh(false);
			Mesh1P->SetHiddenInGame(false);
			Mesh3P->SetHiddenInGame(false);
			Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}

		UE_LOG(LogTemp, Log, TEXT("Switched weapon %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));
	}
}

USkeletalMeshComponent* APreeminentWeapon::GetWeaponMesh() const
{
	//return MeshComp;
	return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

void APreeminentWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

void APreeminentWeapon::OnUnEquip()
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

void APreeminentWeapon::SetOwningPawn(APreeminentCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

float APreeminentWeapon::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void APreeminentWeapon::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		//UAnimMontage* UseAnim = Animation.Pawn1P;

		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

void APreeminentWeapon::Fire()
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

bool APreeminentWeapon::ServerStartFire_Validate()
{
	return true;
}

void APreeminentWeapon::ServerStartFire_Implementation()
{
	BeginFiring();
}

bool APreeminentWeapon::ServerStopFire_Validate()
{
	return true;
}

void APreeminentWeapon::ServerStopFire_Implementation()
{
	StopFiring();
}

void APreeminentWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

void APreeminentWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool APreeminentWeapon::ServerStartReload_Validate()
{
	return true;
}

bool APreeminentWeapon::ServerStopReload_Validate()
{
	return true;
}

void APreeminentWeapon::BeginFiring()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}
	
	float FirstDelay = FMath::Max(LastFiredTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &APreeminentWeapon::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);
}

void APreeminentWeapon::StopFiring()
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

void APreeminentWeapon::StartReload()
{
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	MagazineSize = MagazineCapacity;
}

void APreeminentWeapon::StopReload()
{
	// TODO: Implement...
}

void APreeminentWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, GetWeaponMesh(), MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = GetWeaponMesh()->GetSocketLocation(MuzzleSocketName);

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

void APreeminentWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
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
		FVector MuzzleLocation = GetWeaponMesh()->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void APreeminentWeapon::ServerFire_Implementation()
{
	Fire();
}

bool APreeminentWeapon::ServerFire_Validate()
{
	// This is where anti-cheat should happen.
	return true;
}

void APreeminentWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX

	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
	PlayWeaponAnimation(FireAnim);
}


void APreeminentWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APreeminentWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APreeminentWeapon, ADSAmount, COND_SkipOwner);
}
