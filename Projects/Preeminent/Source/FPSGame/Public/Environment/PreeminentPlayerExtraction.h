// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PreeminentPlayerExtraction.generated.h"

/**
 * Area for where Players can go to extract from a level
 */
UCLASS()
class PREEMINENT_API APreeminentPlayerExtraction : public ATriggerBox
{
	GENERATED_BODY()

	/** Number of Players in this player extraction zone*/
	UPROPERTY(replicated)
	uint8 NumPlayersInZone;

public:
	APreeminentPlayerExtraction();

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
		void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

public:
	//////////////////////////////////////////////////////////////////////////
	// Accessors

	/** Accessor for the number of Players currently inside of this player extracton zone*/
	uint8 GetNumberOfPlayersInZone() const;

};
