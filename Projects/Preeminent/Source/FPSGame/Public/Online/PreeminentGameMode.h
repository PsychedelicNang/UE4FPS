// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PreeminentGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API APreeminentGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	class APreeminentPlayerExtraction* PlayerExtraction;
	class APreeminentLootExtraction* LootExtraction;
	class APreeminentStockpile* Stockpile;

	///** default inventory list */
	//UPROPERTY(EditDefaultsOnly, Category = "Objectives")
	//	TSubclassOf<class APreeminentStockpile> StockpilePrefab;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumPlayersRequiredToExtract;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumLootBagsRequiredToExtract;

protected:
	void CheckGameStatus();

	void RestartDeadPlayers();

public:
	APreeminentGameMode();

public:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	bool RequestRestartDeadPlayers();
};
