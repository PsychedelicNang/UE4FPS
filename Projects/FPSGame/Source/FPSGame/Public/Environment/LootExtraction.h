// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "LootExtraction.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API ALootExtraction : public ATriggerBox
{
	GENERATED_BODY()

	uint8 NumBagsInZone;

public:
	ALootExtraction();

protected:

	virtual void BeginPlay() override;

	// declare overlap begin function
	UFUNCTION()
		void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	// declare overlap end function
	UFUNCTION()
		void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

};
