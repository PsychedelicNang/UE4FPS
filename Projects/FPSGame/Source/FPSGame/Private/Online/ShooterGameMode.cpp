// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "ShooterCharacter.h"
#include "LootExtraction.h"
#include "PlayerExtraction.h"
#include "Kismet/GameplayStatics.h"

AShooterGameMode::AShooterGameMode()
{
	NumPlayersRequiredToExtract = 1;
	NumLootBagsRequiredToExtract = 1;

	PrimaryActorTick.bCanEverTick = true;

	// Only tick once a second
	PrimaryActorTick.TickInterval = 1.0f;
}

void AShooterGameMode::StartPlay()
{
	Super::StartPlay();

	// Find all of the PlayerExtractions in the level
	TArray<AActor*> FoundPlayerExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerExtraction::StaticClass(), FoundPlayerExtractions);

	if (FoundPlayerExtractions.Num())
	{
		PlayerExtraction = Cast<APlayerExtraction>(FoundPlayerExtractions[0]);
	}

	// Find all of the LootExtractions in the level
	TArray<AActor*> FoundLootExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALootExtraction::StaticClass(), FoundLootExtractions);

	if (FoundLootExtractions.Num())
	{
		LootExtraction = Cast<ALootExtraction>(FoundLootExtractions[0]);
	}
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckGameStatus();
}

void AShooterGameMode::CheckGameStatus()
{
	// If the enemy team is dead, and at least RequiredLootBags are extracted, allow for players to extract

	if (PlayerExtraction && LootExtraction)
	{
		uint8 NumPlayersInZone = PlayerExtraction->GetNumberOfPlayersInZone();
		uint8 NumLootBagsInZone = LootExtraction->GetNumberOfLootBagsInZone();
		if (NumPlayersInZone >= NumPlayersRequiredToExtract && NumLootBagsInZone >= NumLootBagsRequiredToExtract)
		{
			UE_LOG(LogTemp, Log, TEXT("GAME OVER!"));
		}
	}
}