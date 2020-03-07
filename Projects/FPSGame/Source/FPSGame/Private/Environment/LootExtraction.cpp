// Fill out your copyright notice in the Description page of Project Settings.


#include "LootExtraction.h"
#include "Containers/LootBag.h"
#include "DrawDebugHelpers.h"

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

void ALootExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
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

uint8 ALootExtraction::GetNumberOfLootBagsInZone() const
{
	return NumBagsInZone;
}
