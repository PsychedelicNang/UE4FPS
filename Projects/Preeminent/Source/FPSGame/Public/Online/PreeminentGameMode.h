// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PreeminentGameMode.generated.h"

/**
 * Default GameMode in Preeminent
 */
UCLASS()
class PREEMINENT_API APreeminentGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	/*Player extraction in the level*/
	class APreeminentPlayerExtraction* PlayerExtraction;

	/*Loot extraction in the level*/
	class APreeminentLootExtraction* LootExtraction;
	
	/*Stockpile in the level*/
	class APreeminentStockpile* Stockpile;

	/*Number of players which must be in the player extraction in order to initiate the extraction*/
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumPlayersRequiredToExtract;
	
	/*Number of loot bags which must be in the loot extraction in order to initiate the extraction*/
	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	uint8 NumLootBagsRequiredToExtract;

protected:
	/*Check our winning conditions and then update all players*/
	void CheckGameStatus();

	/* Restarts all dead players.
	* @warning This is only to be used in development!*/
	void RestartDeadPlayers();

public:
	APreeminentGameMode();

public:
	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	/* Requests to restart all dead players.
	* @warning This is only to be used in development!*/
	UFUNCTION(BlueprintCallable)
	bool RequestRestartDeadPlayers();
};
