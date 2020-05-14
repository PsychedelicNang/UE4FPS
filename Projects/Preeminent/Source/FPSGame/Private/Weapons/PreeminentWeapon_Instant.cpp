// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentWeapon_Instant.h"
#include "Preeminent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AShooterWeapon_Instant::AShooterWeapon_Instant(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//PrimaryActorTick.bCanEverTick = true;
}

void AShooterWeapon_Instant::FireWeapon()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		//// Bullet spread
		//float HalfRad = FMath::DegreesToRadians(BulletSpread);
		//ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
		FVector MuzzleLocation = GetWeaponMesh()->GetSocketLocation(TracerStartName);
		FVector ShotDirection = GetWeaponMesh()->GetSocketRotation(TracerStartName).Vector();
		
		FVector TraceEnd = MuzzleLocation + (EyeRotation.Vector() * InstantConfig.WeaponRange);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true; // Traces against every triangle of the mesh we hit
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		bool bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLocation, TraceEnd, COLLISION_WEAPON, QueryParams);
		if (bBlockingHit)
		{
			// We hit something! Deal damage...

			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = InstantConfig.HitDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage = InstantConfig.HeadshotDamage;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;
		}

		//if (DebugWeaponDrawing > 0)
		//{
			DrawDebugLine(GetWorld(), MuzzleLocation, TraceEnd, FColor::Green, false, 1.0f, 0, 1.0f);
		//}

		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFiredTime = GetWorld()->TimeSeconds;

		PlayWeaponAnimation(FireAnim);
	}
}

void AShooterWeapon_Instant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//FVector MuzzleLocation = GetWeaponMesh()->GetSocketLocation(TracerStartName);
	//DrawDebugSphere(GetWorld(), MuzzleLocation, .5f, 2, FColor::Yellow, false, 0, 0, 1.0f);
}
