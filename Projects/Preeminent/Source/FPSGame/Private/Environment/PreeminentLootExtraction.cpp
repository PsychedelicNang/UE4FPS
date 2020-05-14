// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentLootExtraction.h"
#include "Interactables/PreeminentLootBag.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

ALootExtraction::ALootExtraction()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &ALootExtraction::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &ALootExtraction::OnOverlapEnd);
}

void ALootExtraction::BeginPlay()
{
	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Cyan, true, -1, 0, 5);
}

void ALootExtraction::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of bags in the zone if we are the server. The number of bags should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			ALootBag* LootBag = Cast<ALootBag>(OtherActor);
			if (LootBag)
			{
				++NumBagsInZone;

				// Set the loot bag's position to the center of the extraction zone and then stop the loot bag from continuing to move
				LootBag->SetActorLocation(GetActorLocation());
				LootBag->StopMovementAndDrop();
			}
		}
	}
}

void ALootExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of bags in the zone if we are the server. The number of bags should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			ALootBag* LootBag = Cast<ALootBag>(OtherActor);
			if (LootBag)
			{
				--NumBagsInZone;
			}
		}
	}
}

uint8 ALootExtraction::GetNumberOfLootBagsInZone() const
{
	return NumBagsInZone;
}

void ALootExtraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootExtraction, NumBagsInZone);
}
