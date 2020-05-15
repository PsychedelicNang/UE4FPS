// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PreeminentHealthComponent.generated.h"

/** OnHealthChanged event*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UPreeminentHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);


/**
 * The component for handling the default Health system in Preeminent
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PREEMINENT_API UPreeminentHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties*/
	UPreeminentHealthComponent();

private:
	/** Should always be true when Health is 0.0f. True means this component is dead, but does not control whether the pawn is dead or alive*/
	bool bIsDead;

protected:
	/** Health of this component. Used to determine whether the player is dead or alive*/
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
		float Health;

	/** DefaulHealth will be used if HealthPartitions is null*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
		float DefaultHealth;

	/** List of Health values which will represent the maximum "heal to" values for the player*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	TArray<int32> HealthPartitions;

	/** Current HealthPartition index*/
	UPROPERTY(replicated)
	uint8 HealthPartitionIndex;

	/** What team this player belongs to*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
		uint8 TeamNum;

protected:
	/** Called when the game starts*/
	virtual void BeginPlay() override;

	/** Called when the owner of this component is damaged*/
	UFUNCTION()
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/** [client] handle health changed*/
	UFUNCTION()
		void OnRep_Health(float OldHealth);

	/** [server] heal this health component*/
	UFUNCTION(server, reliable, WithValidation)
	void ServerHeal(float HealAmount);

	/**[server] kill the pawn we're attached to
	* @warning This should only be used during development. This is not intended for builds.
	*/
	UFUNCTION(server, reliable, WithValidation)
	void ServerKillSelf();

public:
	/** Event for when Health is changed*/
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnHealthChangedSignature OnHealthChanged;

	//////////////////////////////////////////////////////////////////////////
	// Accessors

	/* Heal this health component. Clamp our result to the current HealthParition, if using one
	* @param HealAmount - The amount of Health wanting to be added to our current Health amount
	*/
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		void KillSelf();

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		uint8 GetTeamNumber() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		TArray<int32> GetHealthPartitions() const;
	
	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
		uint8 GetHealthPartitionIndex() const;

	/** Determine if the two Actors are on the same team*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
		static bool IsFriendly(AActor* ActorA, AActor* ActorB);
};
