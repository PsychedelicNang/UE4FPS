// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreeminentWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UAnimMontage;

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

// Contains information of a single hitscan weapon linetrace
USTRUCT()
struct FHitScanTrace {
	GENERATED_BODY()

public:
	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
		FVector_NetQuantize TraceTo;
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn1P;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* Pawn3P;
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** inifite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	bool bInfiniteAmmo;

	/** infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		bool bInfiniteClip;

	/** max ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 MaxAmmo;

	/** clip size */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 AmmoPerClip;

	/** initial clips */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
		int32 InitialClips;

	/** time between two consecutive shots (TimeBetweenShots = 60 / RPM) */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float TimeBetweenShots;

	/** failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float NoAnimReloadDuration;

	/** the time it takes to fully aim down sight with this weapon */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float AimDownSightTime;

	/** defaults */
	FWeaponData()
	{
		bInfiniteAmmo = false;
		bInfiniteClip = false;
		MaxAmmo = 240;
		AmmoPerClip = 30;
		InitialClips = 8;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
		AimDownSightTime = 0.0f;
	}
};

UCLASS(Abstract, Blueprintable)
class PREEMINENT_API APreeminentWeapon : public AActor
{
	GENERATED_UCLASS_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", Replicated)
	float ADSAmount;

//public:	
//	// Sets default values for this actor's properties
//	APreeminentWeapon();
//
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh1P;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh3P;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* TracerEffect;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerStartName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<UCameraShake> FireCamShake;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFiredTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		int32 MagazineCapacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		int32 MagazineSize;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
		// Bullet spread in degrees
		float BulletSpread;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		FWeaponAnim FireAnim;

public:
	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
		FWeaponData WeaponConfig;
protected:
	/** current weapon state */
	EWeaponState::Type CurrentState;

	/** spawned component for muzzle FX */
	UPROPERTY(Transient)
		class UParticleSystemComponent* MuzzlePSC;

protected:
	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	virtual void FireWeapon() PURE_VIRTUAL(AShooter::FireWeapon, );

	// Server means it will push the request from client to server. Reliable means it WILL eventually happen. WithValidation is required when using Server
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	virtual void BeginPlay() override;

	UFUNCTION()
		void OnRep_HitScanTrace();

	/** play weapon animations */
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	/** stop playing weapon animations */
	void StopWeaponAnimation(const FWeaponAnim& Animation);

	/** pawn owner */
	UPROPERTY(Transient)
	class APreeminentCharacter* MyPawn;

	///** [server] weapon was added to pawn's inventory */
	//virtual void OnEnterInventory(APreeminentCharacter* NewOwner);

	///** [server] weapon was removed from pawn's inventory */
	//virtual void OnLeaveInventory();

		//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(reliable, server, WithValidation)
		void ServerStartFire();

	UFUNCTION(reliable, server, WithValidation)
		void ServerStopFire();

	UFUNCTION(reliable, server, WithValidation)
		void ServerStartReload();

	UFUNCTION(reliable, server, WithValidation)
		void ServerStopReload();


	//////////////////////////////////////////////////////////////////////////
// Inventory

/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();

	bool bPendingEquip;
	bool bIsEquipped;

	/** get weapon mesh (needs pawn owner to determine variant) */
	USkeletalMeshComponent* GetWeaponMesh() const;


public:
	virtual void BeginFiring();
	virtual void StopFiring();
	virtual void StartReload();
	virtual void StopReload();

	/** set the weapon's owning pawn */
	void SetOwningPawn(APreeminentCharacter* APreeminentCharacter);

	EWeaponState::Type GetCurrentState() const;

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip(const APreeminentWeapon* LastWeapon);

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(APreeminentCharacter* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	bool IsAttachedToPawn() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		float GetADSAmount () const;
};
