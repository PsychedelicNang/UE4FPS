// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	class APlayerExtraction* PlayerExtraction;
	class ALootExtraction* LootExtraction;
	class AStockpile* Stockpile;

	///** default inventory list */
	//UPROPERTY(EditDefaultsOnly, Category = "Objectives")
	//	TSubclassOf<class AStockpile> StockpilePrefab;

	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumPlayersRequiredToExtract;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumLootBagsRequiredToExtract;

protected:
	void CheckGameStatus();

	void RestartDeadPlayers();

public:
	AShooterGameMode();

public:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	bool RequestRestartDeadPlayers();
};
