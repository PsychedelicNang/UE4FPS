// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PreeminentPlayerController.generated.h"

/**
 * Player controller for main character (Pawn) in Preeminent
 */
UCLASS()
class PREEMINENT_API APreeminentPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/* Event for when the mission has been completed*/
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerController")
		void OnMissionComplete();
};
