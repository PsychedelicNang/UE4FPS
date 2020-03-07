// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Stockpile.generated.h"

class ALootBag;

/**
 * 
 */
UCLASS()
class FPSGAME_API AStockpile : public ATriggerBox
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<ALootBag> LootBagToSpawn;
	
	UPROPERTY(EditDefaultsOnly)
		uint8 NumLootBagsToSpawn;

	TArray<ALootBag*> LootBagPool;

public:
	AStockpile();

	virtual void BeginPlay() override;
};
