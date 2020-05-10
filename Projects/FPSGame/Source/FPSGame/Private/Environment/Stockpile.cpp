// Fill out your copyright notice in the Description page of Project Settings.


#include "Stockpile.h"
#include "LootBag.h"
#include "DrawDebugHelpers.h"
#include "Components/ShapeComponent.h"
#include "ShooterCharacter.h"
#include "Net/UnrealNetwork.h"

AStockpile::AStockpile()
{
	NumLootBagsToSpawn = 3;

	//(X=-730.000000,Y=-600.000000,Z=440.000000)
	SetReplicates(true);
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

	// Only spawn the LootBags if we are the server
	if (Role == ROLE_Authority)
	{
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

	// Make sure physics bodies (Pawns) do not go into the trigger box

	UShapeComponent* CollisionComponent = GetCollisionComponent();
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	//CollisionComponent->SetCollisionResponseToChannel()
}

void AStockpile::GetLootBagFromPile(AShooterCharacter* Requester)
{
	UE_LOG(LogTemp, Log, TEXT("Getting loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));

	//Instigator = Requester;
	// net owner for RPC calls

	if (Role == ROLE_Authority)
	{
		SetOwner(Requester);
		
		//if (LootBagPool.Num() > 0)
		//{
		//	if (Requester)
		//	{
		//		UE_LOG(LogTemp, Log, TEXT("Got loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));
		//		ALootBag* LootBag = LootBagPool.Pop();
		//		Requester->EquipLootBag(LootBag);
		//	}
		//	else
		//	{
		//		UE_LOG(LogTemp, Log, TEXT("Shit was NULL man"));
		//	}
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Log, TEXT("Shit was empty man"));
		//}
	}
	//else
	//{
		if (LootBagPool.Num() > 0)
		{
			if (Requester)
			{
				UE_LOG(LogTemp, Log, TEXT("Got loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));
				ALootBag* LootBag = LootBagPool.Pop();
				Requester->EquipLootBag(LootBag);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Shit was NULL man"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Shit was empty man"));
		}
		//UE_LOG(LogTemp, Log, TEXT("Not loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));
		//ServerRequestLootBag(Requester);
	//}

	//if (LootBagPool.Num() > 0)
	//{
	//	if (Requester)
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("Got loot bag %s"), (Role == ROLE_Authority) ? TEXT("True") : TEXT("False"));
	//		ALootBag* LootBag = LootBagPool.Pop();
	//		Requester->EquipLootBag(LootBag);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("Shit was NULL man"));
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Shit was empty man"));
	//}
}

bool AStockpile::ServerRequestLootBag_Validate(AShooterCharacter* Requester)
{
	return true;
}

void AStockpile::ServerRequestLootBag_Implementation(AShooterCharacter* Requester)
{
	GetLootBagFromPile(Requester);
}

void AStockpile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStockpile, LootBagPool);
}
