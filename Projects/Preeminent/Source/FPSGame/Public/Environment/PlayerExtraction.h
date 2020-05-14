// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PlayerExtraction.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API APlayerExtraction : public ATriggerBox
{
	GENERATED_BODY()

	UPROPERTY(replicated)
	uint8 NumPlayersInZone;

public:
	APlayerExtraction();

protected:

	virtual void BeginPlay() override;

	// declare overlap begin function
	UFUNCTION()
		void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	// declare overlap end function
	UFUNCTION()
		void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

public:
	uint8 GetNumberOfPlayersInZone() const;

};
