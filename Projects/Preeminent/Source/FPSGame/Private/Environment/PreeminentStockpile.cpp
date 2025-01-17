// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentStockpile.h"
#include "PreeminentLootBag.h"
#include "DrawDebugHelpers.h"
#include "Components/ShapeComponent.h"
#include "PreeminentCharacter.h"
#include "Net/UnrealNetwork.h"

APreeminentStockpile::APreeminentStockpile()
{
	NumLootBagsToSpawn = 3;

	SetReplicates(true);
}

void APreeminentStockpile::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Red, true, -1, 0, 5);

	if (LootBagToSpawn == nullptr)
	{
		return;
	}

	FVector StockpileExtents = GetComponentsBoundingBox().GetExtent();
	float XExtent = StockpileExtents.X / 2.0f;

	// Only spawn the LootBags if we are the server
	if (Role == ROLE_Authority)
	{
		float Offset = 0.0f;

		for (uint8 i = 0; i < NumLootBagsToSpawn; i++)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			APreeminentLootBag* Spawned = GetWorld()->SpawnActor<APreeminentLootBag>(LootBagToSpawn, SpawnInfo);
			FVector Extent = Spawned->GetMeshExtents();
			Spawned->SetActorLocation(GetActorLocation() + FVector(Offset - XExtent, 0.0f, 0.0f));
			LootBagPool.Add(Spawned);

			Offset += Extent.X;
		}
	}

	// Make sure physics bodies (Pawns) do not go into the trigger box

	UShapeComponent* CollisionComponent = GetCollisionComponent();
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	SetOwner(GetWorld()->GetFirstPlayerController());
}

void APreeminentStockpile::GetLootBagFromPile(APreeminentCharacter* Requester)
{
	UE_LOG(LogTemp, Log, TEXT("Getting loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));

	if (Role < ROLE_Authority)
	{
		// net owner for RPC calls
		ServerRequestLootBag(Requester);
	}

	if (LootBagPool.Num() > 0)
	{
		if (Requester)
		{
			UE_LOG(LogTemp, Log, TEXT("Got loot bag %s %d"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"), LootBagPool.Num());
			APreeminentLootBag* LootBag = LootBagPool.Pop();
			Requester->EquipLootBag(LootBag);
		}
	}
}

bool APreeminentStockpile::ServerRequestLootBag_Validate(APreeminentCharacter* Requester)
{
	return true;
}

void APreeminentStockpile::ServerRequestLootBag_Implementation(APreeminentCharacter* Requester)
{
	GetLootBagFromPile(Requester);
}

void APreeminentStockpile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APreeminentStockpile, LootBagPool, COND_SkipOwner);
}
