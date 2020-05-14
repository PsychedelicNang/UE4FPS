// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ShooterHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UShooterHealthComponent::UShooterHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;

	SetIsReplicated(true);

	TeamNum = 255;
	HealthPartitionIndex = 0;
}

// Called when the game starts
void UShooterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UShooterHealthComponent::HandleTakeAnyDamage);
		}
	}

	if (HealthPartitions.Num() > 0)
	{
		Health = HealthPartitions[HealthPartitionIndex];
	}
	else
	{
		Health = DefaultHealth;
	}
}

// Called every frame
void UShooterHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UShooterHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage <= 0.0f || bIsDead)
	{
		return;
	}

	//if (DamageCauser != DamagedActor /*&& IsFriendly(DamagedActor, DamageCauser)*/)
	//{
	//	return;
	//}

	// Update health clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, (float)HealthPartitions[HealthPartitionIndex]);

	// If we fall below a health parition, update the max health we can heal to
	if (HealthPartitionIndex != HealthPartitions.Num() - 1)
	{
		if (Health <= (float)HealthPartitions[HealthPartitionIndex + 1])
		{
			++HealthPartitionIndex;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s %s (HealthPartitionIndex == %d)"), *FString::SanitizeFloat(Health), (GetOwnerRole() == ROLE_Authority) ? TEXT("True") : TEXT("False"), HealthPartitionIndex);

	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (Health <= 0.0f)
	{
		//ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		//if (GM)
		//{
		//	GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		//}
	}
}

void UShooterHealthComponent::OnRep_Health(float OldHealth)
{
	float HealthDelta = Health - OldHealth;
	UE_LOG(LogTemp, Log, TEXT("OnRep_Health (HealthDelta): %s %s"), *FString::SanitizeFloat(HealthDelta), (GetOwnerRole() == ROLE_Authority) ? TEXT("True") : TEXT("False"));

	OnHealthChanged.Broadcast(this, Health, HealthDelta, nullptr, nullptr, nullptr);
}

void UShooterHealthComponent::ServerHeal_Implementation(float HealAmount)
{
	Heal(HealAmount);
}

bool UShooterHealthComponent::ServerHeal_Validate(float HealAmount)
{
	return true;
}

void UShooterHealthComponent::Heal(float HealAmount)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (HealAmount <= 0.0f || Health <= 0.0f)
		{
			return;
		}

		Health = FMath::Clamp(Health + HealAmount, 0.0f, (float)HealthPartitions[HealthPartitionIndex]);

		OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
	}
	else
	{
		ServerHeal(HealAmount);
	}
}

// For testing only! Remove in official builds...
void UShooterHealthComponent::KillSelf()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		float Damage = Health;
		Health = 0.0f;
		bIsDead = Health <= 0.0f;

		AActor* MyOwner = GetOwner();
		OnHealthChanged.Broadcast(this, Health, Damage, nullptr, UGameplayStatics::GetPlayerController(MyOwner, 0), MyOwner);
	}
	else
	{
		ServerKillSelf();
	}
}

bool UShooterHealthComponent::ServerKillSelf_Validate()
{
	return true;
}

void UShooterHealthComponent::ServerKillSelf_Implementation()
{
	KillSelf();
}

float UShooterHealthComponent::GetHealth() const
{
	return Health;
}

TArray<int32> UShooterHealthComponent::GetHealthPartitions() const
{
	return HealthPartitions;
}

uint8 UShooterHealthComponent::GetHealthPartitionIndex() const
{
	return HealthPartitionIndex;
}

bool UShooterHealthComponent::IsFriendly(AActor * ActorA, AActor * ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		// Assume friendly
		return true;
	}

	UShooterHealthComponent* HealthCompA = Cast<UShooterHealthComponent>(ActorA->GetComponentByClass(UShooterHealthComponent::StaticClass()));
	UShooterHealthComponent* HealthCompB = Cast<UShooterHealthComponent>(ActorB->GetComponentByClass(UShooterHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume friendly
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

void UShooterHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShooterHealthComponent, Health);
	DOREPLIFETIME(UShooterHealthComponent, HealthPartitionIndex);
}