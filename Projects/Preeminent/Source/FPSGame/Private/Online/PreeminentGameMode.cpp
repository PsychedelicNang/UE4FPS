// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentGameMode.h"
#include "PreeminentCharacter.h"
#include "PreeminentLootExtraction.h"
#include "PreeminentPlayerExtraction.h"
#include "Kismet/GameplayStatics.h"
#include "PreeminentGameState.h"
#include "PreeminentPlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "PreeminentStockpile.h"

APreeminentGameMode::APreeminentGameMode()
{
	NumPlayersRequiredToExtract = 1;
	NumLootBagsRequiredToExtract = 1;

	PrimaryActorTick.bCanEverTick = true;

	// Only tick once a second
	PrimaryActorTick.TickInterval = 1.0f;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnObj(TEXT("/Game/Blueprints/Pawns/BP_PlayerPawn"));
	DefaultPawnClass = PlayerPawnObj.Class;

	GameStateClass = APreeminentGameState::StaticClass();
	PlayerControllerClass = APreeminentPlayerController::StaticClass();
}

void APreeminentGameMode::StartPlay()
{
	Super::StartPlay();
	
	/*Find all game objects needed for this game mode*/

	// Find all of the PlayerExtractions in the level
	TArray<AActor*> FoundPlayerExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APreeminentPlayerExtraction::StaticClass(), FoundPlayerExtractions);

	if (FoundPlayerExtractions.Num() > 0)
	{
		PlayerExtraction = Cast<APreeminentPlayerExtraction>(FoundPlayerExtractions[0]);
		//PlayerExtraction->SetOwner(this);
	}

	// Find all of the LootExtractions in the level
	TArray<AActor*> FoundLootExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APreeminentLootExtraction::StaticClass(), FoundLootExtractions);

	if (FoundLootExtractions.Num() > 0)
	{
		LootExtraction = Cast<APreeminentLootExtraction>(FoundLootExtractions[0]);
		//LootExtraction->SetOwner(this);
	}

	// Find all of the Stockpiles in the level
	TArray<AActor*> FoundStockpiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APreeminentStockpile::StaticClass(), FoundStockpiles);

	if (FoundStockpiles.Num() > 0)
	{
		Stockpile = Cast<APreeminentStockpile>(FoundStockpiles[0]);
		//Stockpile->SetOwner(this);
	}
}

void APreeminentGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckGameStatus();
}

bool APreeminentGameMode::RequestRestartDeadPlayers()
{
	// TODO: Implement conditions
	if (true)
	{
		RestartDeadPlayers();

		return true;
	}
	return false;
}

void APreeminentGameMode::CheckGameStatus()
{
	// If the enemy team is dead, and at least RequiredLootBags are extracted, allow for players to extract

	if (PlayerExtraction && LootExtraction)
	{
		uint8 NumPlayersInZone = PlayerExtraction->GetNumberOfPlayersInZone();
		uint8 NumLootBagsInZone = LootExtraction->GetNumberOfLootBagsInZone();
		if (NumPlayersInZone >= NumPlayersRequiredToExtract && NumLootBagsInZone >= NumLootBagsRequiredToExtract)
		{
			APreeminentGameState* GS = GetGameState<APreeminentGameState>();
			if (GS)
			{
				GS->MulticastOnMissionComplete();
			}
			UE_LOG(LogTemp, Log, TEXT("GAME OVER!"));
		}
	}
}

void APreeminentGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}