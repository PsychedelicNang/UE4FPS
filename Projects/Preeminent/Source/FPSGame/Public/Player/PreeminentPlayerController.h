// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PreeminentPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PREEMINENT_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
		void OnMissionComplete();
};
