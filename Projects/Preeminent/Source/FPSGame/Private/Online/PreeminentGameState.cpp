// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentGameState.h"
#include "PreeminentPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PreeminentGameMode.h"

void APreeminentGameState::MulticastOnMissionComplete_Implementation()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APreeminentPlayerController* PC = Cast<APreeminentPlayerController>(It->Get());
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
