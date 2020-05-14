// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentPlayerExtraction.h"
#include "DrawDebugHelpers.h"
#include "PreeminentCharacter.h"
#include "Net/UnrealNetwork.h"

APlayerExtraction::APlayerExtraction()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &APlayerExtraction::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &APlayerExtraction::OnOverlapEnd);
}

void APlayerExtraction::BeginPlay()
{
	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Green, true, -1, 0, 5);
}

void APlayerExtraction::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of players in the zone if we are the server. The number of players should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
			if (ShooterCharacter)
			{
				++NumPlayersInZone;
			}
		}
	}
}

void APlayerExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of players in the zone if we are the server. The number of players should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
			if (ShooterCharacter)
			{
				--NumPlayersInZone;
			}
		}
	}
}

uint8 APlayerExtraction::GetNumberOfPlayersInZone() const
{
	return NumPlayersInZone;
}

void APlayerExtraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerExtraction, NumPlayersInZone);
}