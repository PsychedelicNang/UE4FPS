// Fill out your copyright notice in the Description page of Project Settings.


#include "Stockpile.h"
#include "LootBag.h"
#include "DrawDebugHelpers.h"

AStockpile::AStockpile()
{
	NumLootBagsToSpawn = 3;
}

void AStockpile::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Red, true, -1, 0, 5);

	if (LootBagToSpawn == nullptr)
	{
		return;
	}

	FVector StockpileExtents = GetComponentsBoundingBox().GetExtent();
	float XExtent = StockpileExtents.X / 2.0f;

	float Offset = 0.0f;

	for (uint8 i = 0; i < NumLootBagsToSpawn; i++)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		ALootBag* Spawned = GetWorld()->SpawnActor<ALootBag>(LootBagToSpawn, SpawnInfo);
		FVector Extent = Spawned->GetMeshExtents();
		Spawned->SetActorLocation(GetActorLocation() + FVector(Offset - XExtent, 0.0f, 0.0f));
		LootBagPool.Add(Spawned);
		
		Offset += Extent.X;
	}
}