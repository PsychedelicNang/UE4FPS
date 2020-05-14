// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentCharacterMovement.h"
#include "PreeminentCharacter.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UPreeminentCharacterMovement::UPreeminentCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


float UPreeminentCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const APreeminentCharacter* ShooterCharacterOwner = Cast<APreeminentCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		else if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
		if (ShooterCharacterOwner->IsCarryingLootBag())
		{
			MaxSpeed *= ShooterCharacterOwner->GetCarryingLootBagSpeedModifier();
		}
	}

	return MaxSpeed;
}
