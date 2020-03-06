// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerExtraction.h"
#include "DrawDebugHelpers.h"
#include "ShooterCharacter.h"

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
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			++NumPlayersInZone;
		}
	}
}

void APlayerExtraction::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
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
