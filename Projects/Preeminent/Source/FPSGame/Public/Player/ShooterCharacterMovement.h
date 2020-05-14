// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	
	virtual float GetMaxSpeed() const override;
};
