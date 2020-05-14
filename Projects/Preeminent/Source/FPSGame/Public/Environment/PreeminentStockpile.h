// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PreeminentStockpile.generated.h"

class APreeminentLootBag;
class APreeminentCharacter;

/**
 * 
 */
UCLASS()
class PREEMINENT_API APreeminentStockpile : public ATriggerBox
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<APreeminentLootBag> LootBagToSpawn;
	
	UPROPERTY(EditDefaultsOnly)
		uint8 NumLootBagsToSpawn;

	UPROPERTY(replicated)
	TArray<APreeminentLootBag*> LootBagPool;

public:
	APreeminentStockpile();

	virtual void BeginPlay() override;

	void GetLootBagFromPile(APreeminentCharacter* Requester);

	UFUNCTION(reliable, server, WithValidation)
		void ServerRequestLootBag(APreeminentCharacter* Requester);
};
