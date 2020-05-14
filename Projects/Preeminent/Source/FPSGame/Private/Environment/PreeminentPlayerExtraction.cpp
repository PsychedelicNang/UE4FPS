// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentPlayerExtraction.h"
#include "DrawDebugHelpers.h"
#include "PreeminentCharacter.h"
#include "Net/UnrealNetwork.h"

APreeminentPlayerExtraction::APreeminentPlayerExtraction()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &APreeminentPlayerExtraction::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &APreeminentPlayerExtraction::OnOverlapEnd);
}

void APreeminentPlayerExtraction::BeginPlay()
{
	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Green, true, -1, 0, 5);
}

void APreeminentPlayerExtraction::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of players in the zone if we are the server. The number of players should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			APreeminentCharacter* ShooterCharacter = Cast<APreeminentCharacter>(OtherActor);
			if (ShooterCharacter)
			{
				++NumPlayersInZone;
			}
		}
	}
}

void APreeminentPlayerExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of players in the zone if we are the server. The number of players should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			APreeminentCharacter* ShooterCharacter = Cast<APreeminentCharacter>(OtherActor);
			if (ShooterCharacter)
			{
				--NumPlayersInZone;
			}
		}
	}
}

uint8 APreeminentPlayerExtraction::GetNumberOfPlayersInZone() const
{
	return NumPlayersInZone;
}

void APreeminentPlayerExtraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APreeminentPlayerExtraction, NumPlayersInZone);
}