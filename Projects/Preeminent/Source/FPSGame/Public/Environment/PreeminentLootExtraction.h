// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PreeminentLootExtraction.generated.h"

/** Area for where Loot can be stored for extraction*/
UCLASS()
class PREEMINENT_API APreeminentLootExtraction : public ATriggerBox
{
	GENERATED_BODY()
	
	/** Number of bags in this Loot Extraction zone*/
	UPROPERTY(replicated)
	uint8 NumBagsInZone;

public:
	APreeminentLootExtraction();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

public:
	//////////////////////////////////////////////////////////////////////////
	// Accessors

	/** Accessor for the number of LootBags currently inside of this Loot Extraction zone*/
	uint8 GetNumberOfLootBagsInZone() const;
};
