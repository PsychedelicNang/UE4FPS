// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

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
	}
};

UCLASS(Abstract, Blueprintable)
class FPSGAME_API AShooterWeapon : public AActor
{
	GENERATED_UCLASS_BODY()
	
//public:	
//	// Sets default values for this actor's properties
//	AShooterWeapon();
//
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;

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

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
		FWeaponData WeaponConfig;

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
	class AShooterCharacter* MyPawn;

	///** [server] weapon was added to pawn's inventory */
	//virtual void OnEnterInventory(AShooterCharacter* NewOwner);

	///** [server] weapon was removed from pawn's inventory */
	//virtual void OnLeaveInventory();

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
	virtual void Reload();

	/** set the weapon's owning pawn */
	void SetOwningPawn(AShooterCharacter* AShooterCharacter);

	EWeaponState::Type GetCurrentState() const;

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip(const AShooterWeapon* LastWeapon);

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(AShooterCharacter* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	bool IsAttachedToPawn() const;
};
