// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	class APlayerExtraction* PlayerExtraction;
	class ALootExtraction* LootExtraction;

	uint8 NumPlayersRequiredToExtract;
	uint8 NumLootBagsRequiredToExtract;

protected:
	void CheckGameStatus();

public:
	AShooterGameMode();

public:
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};
