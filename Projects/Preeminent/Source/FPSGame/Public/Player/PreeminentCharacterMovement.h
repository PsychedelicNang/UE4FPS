// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PreeminentCharacterMovement.generated.h"

/**
 * Character movement for main Preeminent character (Pawn)
 */
UCLASS()
class PREEMINENT_API UPreeminentCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	
	/* determines the max speed based off of the current status of the player*/
	virtual float GetMaxSpeed() const override;
};
