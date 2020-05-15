// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PreeminentGameState.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API APreeminentGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	/*Multicast for when the mission is completed*/
	UFUNCTION(NetMulticast, Reliable)
		void MulticastOnMissionComplete();
};
