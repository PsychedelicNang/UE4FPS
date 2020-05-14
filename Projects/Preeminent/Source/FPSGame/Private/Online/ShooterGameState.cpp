// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterGameMode.h"

void AShooterGameState::MulticastOnMissionComplete_Implementation()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->OnMissionComplete();

			// Disable input
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				Pawn->DisableInput(nullptr);
			}
		}
	}
}
