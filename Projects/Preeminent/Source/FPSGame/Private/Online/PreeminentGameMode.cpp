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

AShooterGameMode::AShooterGameMode()
{
	NumPlayersRequiredToExtract = 1;
	NumLootBagsRequiredToExtract = 1;

	PrimaryActorTick.bCanEverTick = true;

	// Only tick once a second
	PrimaryActorTick.TickInterval = 1.0f;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnObj(TEXT("/Game/Blueprints/Pawns/BP_PlayerPawn"));
	DefaultPawnClass = PlayerPawnObj.Class;

	GameStateClass = AShooterGameState::StaticClass();
	PlayerControllerClass = AShooterPlayerController::StaticClass();
}

void AShooterGameMode::StartPlay()
{
	Super::StartPlay();

	// Find all of the PlayerExtractions in the level
	TArray<AActor*> FoundPlayerExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerExtraction::StaticClass(), FoundPlayerExtractions);

	if (FoundPlayerExtractions.Num() > 0)
	{
		PlayerExtraction = Cast<APlayerExtraction>(FoundPlayerExtractions[0]);
		//PlayerExtraction->SetOwner(this);
	}

	// Find all of the LootExtractions in the level
	TArray<AActor*> FoundLootExtractions;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALootExtraction::StaticClass(), FoundLootExtractions);

	if (FoundLootExtractions.Num() > 0)
	{
		LootExtraction = Cast<ALootExtraction>(FoundLootExtractions[0]);
		//LootExtraction->SetOwner(this);
	}

	//FActorSpawnParameters SpawnInfo;
	//SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//FVector Location = FVector(-730.000000, -600.000000, 440.000000);
	//AStockpile* TheStockpile = GetWorld()->SpawnActor<AStockpile>(StockpilePrefab, Location, FRotator::ZeroRotator, SpawnInfo);
	//TheStockpile->SetOwner(this);

	TArray<AActor*> FoundStockpiles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStockpile::StaticClass(), FoundStockpiles);

	if (FoundStockpiles.Num() > 0)
	{
		Stockpile = Cast<AStockpile>(FoundStockpiles[0]);
		//Stockpile->SetOwner(this);
	}
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckGameStatus();
}

bool AShooterGameMode::RequestRestartDeadPlayers()
{
	// TODO: Implement conditions
	if (true)
	{
		RestartDeadPlayers();

		return true;
	}
	return false;
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
			AShooterGameState* GS = GetGameState<AShooterGameState>();
			if (GS)
			{
				GS->MulticastOnMissionComplete();
			}
			UE_LOG(LogTemp, Log, TEXT("GAME OVER!"));
		}
	}
}

void AShooterGameMode::RestartDeadPlayers()
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