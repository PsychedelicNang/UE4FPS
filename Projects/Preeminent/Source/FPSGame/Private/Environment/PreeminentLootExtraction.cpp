// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentLootExtraction.h"
#include "Interactables/PreeminentLootBag.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

APreeminentLootExtraction::APreeminentLootExtraction()
{
	//Register Events
	OnActorBeginOverlap.AddDynamic(this, &APreeminentLootExtraction::OnOverlapBegin);
	OnActorEndOverlap.AddDynamic(this, &APreeminentLootExtraction::OnOverlapEnd);
}

void APreeminentLootExtraction::BeginPlay()
{
	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Cyan, true, -1, 0, 5);
}

void APreeminentLootExtraction::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of bags in the zone if we are the server. The number of bags should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			APreeminentLootBag* LootBag = Cast<APreeminentLootBag>(OtherActor);
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

void APreeminentLootExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	// Only update the number of bags in the zone if we are the server. The number of bags should be the same at all times for servers and clients, but let's be authoritative to be safe.
	if (Role == ROLE_Authority)
	{
		if (OtherActor)
		{
			APreeminentLootBag* LootBag = Cast<APreeminentLootBag>(OtherActor);
			if (LootBag)
			{
				--NumBagsInZone;
			}
		}
	}
}

uint8 APreeminentLootExtraction::GetNumberOfLootBagsInZone() const
{
	return NumBagsInZone;
}

void APreeminentLootExtraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APreeminentLootExtraction, NumBagsInZone);
}
