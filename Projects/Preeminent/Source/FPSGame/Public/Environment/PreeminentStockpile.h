// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "PreeminentStockpile.generated.h"

class APreeminentLootBag;
class APreeminentCharacter;

/**
 * Area where loot is spawned for players. Generally are only used for the spawning of LootBags or other interactable items which store loot for players
 */
UCLASS()
class PREEMINENT_API APreeminentStockpile : public ATriggerBox
{
	GENERATED_BODY()
	
protected:
	/** The class we will spawn for LootBags*/
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<APreeminentLootBag> LootBagToSpawn;
	
	/** The number of LootBags to spawn*/
	UPROPERTY(EditDefaultsOnly)
		uint8 NumLootBagsToSpawn;

	/** The LootBags which have been spawned and players can grab from this Stockpile*/
	UPROPERTY(replicated)
	TArray<APreeminentLootBag*> LootBagPool;

public:
	APreeminentStockpile();

	virtual void BeginPlay() override;

	/** [client] grabs a LootBag from this stockpile*/
	void GetLootBagFromPile(APreeminentCharacter* Requester);

	/** [server] requests a LootBag from this stockpile and attaches it to the Requester*/
	UFUNCTION(reliable, server, WithValidation)
		void ServerRequestLootBag(APreeminentCharacter* Requester);
};
