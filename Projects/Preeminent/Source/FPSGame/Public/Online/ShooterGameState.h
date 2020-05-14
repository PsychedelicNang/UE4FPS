// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API AShooterGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	UFUNCTION(NetMulticast, Reliable)
		void MulticastOnMissionComplete();
};
