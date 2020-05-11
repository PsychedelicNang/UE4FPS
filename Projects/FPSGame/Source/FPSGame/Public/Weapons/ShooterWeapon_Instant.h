// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon.h"
#include "ShooterWeapon_Instant.generated.h"

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		FVector Origin;

	UPROPERTY()
		float ReticleSpread;

	UPROPERTY()
		int32 RandomSeed;
};

USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

		/** base weapon spread (degrees) */
		UPROPERTY(EditDefaultsOnly, Category = Accuracy)
		float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
		float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
		float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
		float FiringSpreadMax;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		int32 HitDamage;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		int32 HeadshotDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
		TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
		float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
		float AllowedViewDotHitDir;

	/** defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		HeadshotDamage = 40;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

/**
 * 
 */
UCLASS(Abstract)
class FPSGAME_API AShooterWeapon_Instant : public AShooterWeapon
{
	GENERATED_UCLASS_BODY()
	
protected:
	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
		FInstantWeaponData InstantConfig;

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

public:
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
