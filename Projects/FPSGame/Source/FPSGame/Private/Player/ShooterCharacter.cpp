// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/ShooterWeapon.h"
#include "Components/ShooterHealthComponent.h"
#include "FPSGame.h"
#include "Animation/AnimInstance.h"
#include "ShooterCharacterMovement.h"
#include "Containers/LootBag.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Stockpile.h"
#include "TimerManager.h"

// Sets default values
AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	//SpringArmComp->bUsePawnControlRotation = true;
	//SpringArmComp->SetupAttachment(RootComponent);

	// Create a CameraComponent	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComp->SetupAttachment(GetCapsuleComponent());
	CameraComp->RelativeLocation = FVector(0, 0, BaseEyeHeight); // Position the camera
	CameraComp->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComp->SetupAttachment(CameraComp);
	Mesh1PComp->CastShadow = false;
	Mesh1PComp->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	Mesh1PComp->RelativeLocation = FVector(0, 0, -150.0f);

	//// Create a gun mesh component
	//GunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	//GunMeshComp->CastShadow = false;
	//GunMeshComp->SetupAttachment(Mesh1PComp, "GripPoint");

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 65.0f;
	ZoomedInterpSpeed = 20.0f;

	HealthComp = CreateDefaultSubobject<UShooterHealthComponent>(TEXT("HealthComp"));

	bIsTargeting = false;
	bWantsToRun = false;
	bWantsToFire = false;
	bWantsToRunToggled = false;
	bIsCarryingLootBag = false;

	TargetingSpeedModifier = 0.5f;
	RunningSpeedModifier = 1.5f;
	InteractableDistance = 100.0f;

	LaunchStrength = 1000.0f;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled())
	{
		FRotator NewRot = CameraComp->RelativeRotation;
		NewRot.Pitch = RemoteViewPitch * 360.0f/255.0f; // Uncompress RemoteViewPitch

		CameraComp->SetRelativeRotation(NewRot);
	}

	if (bWantsToRunToggled && !IsRunning())
	{
		SetRunning(false, false);
	}

	//float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	//
	//float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomedInterpSpeed);
	//
	//CameraComp->SetFieldOfView(NewFOV);

	//if (Dot())

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController && !bIsCarryingLootBag && CurrentLootBag == nullptr)
	{
		bool QuickCheck = PlayerController->IsInputKeyDown(FKey("F"));
		if (!QuickCheck)
		{
			// Don't do anything if the user is not attemping to iteract with something
			return;
		}

		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);

		//FCollisionObjectQueryParams InterestedInObjects;
		//InterestedInObjects.AddObjectTypesToQuery(ECC_GameTraceChannel2);

		//InterestedInObjects.AddObjectTypesToQuery(ECC_);
		//OBJECTCHANNEL_INTERACTABLE
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(CurrentWeapon);
		TraceParams.AddIgnoredActor(GetOwner());
		TraceParams.AddIgnoredActor(this);
		TraceParams.bTraceComplex = false;
		//TraceParams.bReturnPhysicalMaterial = true;

		FHitResult Hit;

		FVector TraceEnd = CamLoc + (CamRot.Vector() * InteractableDistance);

		//bool bResult = GetWorld()->LineTraceSingleByObjectType(Hit, CamLoc, TraceEnd, InterestedInObjects, TraceParams);
		bool bResult = GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, TraceEnd, COLLISION_INTERACTABLE, TraceParams);

		if (bResult)
		{
			float time = PlayerController->GetInputKeyTimeDown(FKey("F"));
			if (time > 1.0f)
			{
				//bAlreadyGrabbedLootBag = true;
				//GetWorldTimerManager().SetTimer(TimerHandle_GrabLootBagDelay, this, &AShooterCharacter::OnLootBagDelayFinished, 1.0f, false, 0.0f);

				ALootBag* LootBag = Cast<ALootBag>(Hit.Actor);

				if (LootBag)
				{
					CurrentLootBag = LootBag;

					//CurrentWeapon->AttachToComponent(Mesh1PComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachPoint);

					CurrentLootBag->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
					CurrentLootBag->OnEquip();
				
					bIsCarryingLootBag = true;
				}

				AStockpile* Stockpile = Cast<AStockpile>(Hit.Actor);

				if (Stockpile)
				{
					ALootBag* GrabbedBag = Stockpile->GetLootBagFromPile();

					if (GrabbedBag)
					{
						CurrentLootBag = GrabbedBag;
						
						CurrentLootBag->SetOwningPawn(this);
						CurrentLootBag->OnEquip();

						bIsCarryingLootBag = true;

						UE_LOG(LogTemp, Log, TEXT("Grabbed loot bag"));
					}
				}
			}
		}

		//FRotator ActorRot = GetActorRotation();
		//ActorRot.Pitch = CamRot.Pitch;

		//DrawDebugSphere(GetWorld(), GetActorLocation(), 10.0f, 12, FColor::Green, false, 0.0f, 0, 1.0f);
		//DrawDebugSphere(GetWorld(), CamLoc + CamRot.RotateVector(FVector(121, 0, 0)), 10.0f, 12, FColor::Green, false, 0.0f, 0, 1.0f);
	}
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//DefaultFOV = CameraComp->FieldOfView;

	if (HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &AShooterCharacter::OnHealthChanged);
	}

	//if (Role == ROLE_Authority)
	//{
	//	// Spawn a default weapon
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	if (!StarterWeaponClass)
	//	{
	//		return;
	//	}

	//	CurrentWeapon = GetWorld()->SpawnActor<AShooterWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	//	if (CurrentWeapon)
	//	{
	//		CurrentWeapon->SetOwningPawn(this);
	//		CurrentWeapon->AttachToComponent(Mesh1PComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	//		CurrentWeapon->OnWeaponFired.AddDynamic(this, &AShooterCharacter::HandleOnWeaponFired);
	//	}
	//}

	

	
}

void AShooterCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void AShooterCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void AShooterCharacter::BeginCrouch()
{
	Crouch();
}

void AShooterCharacter::EndCrouch()
{
	UnCrouch();
}

void AShooterCharacter::StartJump()
{
	Jump();
}

void AShooterCharacter::StopJump()
{
	StopJumping();
}

void AShooterCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void AShooterCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void AShooterCharacter::OnStartFiring()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{

	if (IsRunning())
	{
		SetRunning(false, false);
	}
	BeginFiring();

	//}
}

void AShooterCharacter::OnStopFiring()
{
	StopFiring();
}

void AShooterCharacter::BeginFiring()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->BeginFiring();
		}
	}
}

void AShooterCharacter::StopFiring()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFiring();
		}
	}
}

bool AShooterCharacter::IsFiring() const
{
	return bWantsToFire;
}

bool AShooterCharacter::IsCarryingLootBag() const
{
	return bIsCarryingLootBag;
}

AShooterWeapon * AShooterCharacter::GetWeapon() const
{
	return CurrentWeapon;
}

bool AShooterCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	//return false;

	return (bWantsToRun || bWantsToRunToggled) && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorForwardVector()) > -0.1;
}

bool AShooterCharacter::IsTargeting() const
{
	return bIsTargeting;
}

void AShooterCharacter::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}

void AShooterCharacter::OnHealthChanged(UShooterHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);
	}
}

void AShooterCharacter::HandleOnWeaponFired(AShooterWeapon * WeaponFired)
{
	//// Get the animation instance on our first person character mesh and play the Weapon Fired Montage
	//UAnimInstance* AnimInstance = (Mesh1PComp) ? Mesh1PComp->GetAnimInstance() : nullptr;
	//if (AnimInstance && WeaponFiredMontage)
	//{
	//	// Play our weapon fired montage
	//	AnimInstance->Montage_Play(WeaponFiredMontage);
	//}
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AShooterCharacter::SpawnDefaultInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AShooterWeapon* NewWeapon = GetWorld()->SpawnActor<AShooterWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (i == 0)
		{
			EquipWeapon(Inventory[i]);
		}
		else
		{
			Inventory[i]->OnUnEquip();
		}
	}


	//if (Role == ROLE_Authority)
	//{
	//	// Spawn a default weapon
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	if (!StarterWeaponClass)
	//	{
	//		return;
	//	}

	//	CurrentWeapon = GetWorld()->SpawnActor<AShooterWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	//	if (CurrentWeapon)
	//	{
	//		CurrentWeapon->SetOwningPawn(this);
	//		CurrentWeapon->AttachToComponent(Mesh1PComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	//		CurrentWeapon->OnWeaponFired.AddDynamic(this, &AShooterCharacter::HandleOnWeaponFired);
	//	}
	//}
}

void AShooterCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	// remove all weapons from inventory and destroy them
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AShooterWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

void AShooterCharacter::AddWeapon(AShooterWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
	}
}

void AShooterCharacter::RemoveWeapon(AShooterWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
	}
}

AShooterWeapon* AShooterCharacter::FindWeapon(TSubclassOf<AShooterWeapon> WeaponClass)
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AShooterWeapon* Weapon)
{
	if (Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
//			ServerEquipWeapon(Weapon);
		}
	}
}

USkeletalMeshComponent* AShooterCharacter::GetSpecifcPawnMesh(bool WantFirstPerson) const
{
	return Mesh1PComp;
	//return WantFirstPerson == true ? Mesh1P : GetMesh();
}

float AShooterCharacter::GetCarryingLootBagSpeedModifier() const
{
	return CurrentLootBag ? CurrentLootBag->GetCarryingLootBagSpeedModifier() : 1.0f;
}

FName AShooterCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

FName AShooterCharacter::GetLootBagAttachPoint() const
{
	return LootBagAttachPoint;
}

void AShooterCharacter::SetCurrentWeapon(AShooterWeapon* NewWeapon, AShooterWeapon* LastWeapon)
{
	AShooterWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon != nullptr)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	//CurrentWeapon->AttachToComponent(Mesh1PComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachPoint);

	// equip new one
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!

		NewWeapon->OnEquip(LastWeapon);
	}
}

void AShooterCharacter::OnNextWeapon()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			AShooterWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon);
		}
	//}
}

void AShooterCharacter::OnPrevWeapon()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			AShooterWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
			EquipWeapon(PrevWeapon);
		}
	//}
}

void AShooterCharacter::OnThrowItem()
{
	// Throw the item being carried (Loot Bag)

	if (bIsCarryingLootBag && CurrentLootBag)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			// Get the location and rotation of the camera
			FVector CamLoc;
			FRotator CamRot;
			PlayerController->GetPlayerViewPoint(CamLoc, CamRot);

			bIsCarryingLootBag = false;
			CurrentLootBag->OnUnEquip();

			// Make sure to move the CurrentLootBag away from our player since we don't want it to collide with the person throwing it.
			FVector NewLocation = CamLoc + CamRot.RotateVector(FVector(120, 0, 0));
			CurrentLootBag->SetActorLocation(NewLocation);
			
			// Launch the lootbag away from the player relative to the camera's rotation
			FVector Direction = CamRot.Vector() * LaunchStrength;
			CurrentLootBag->LaunchItem(Direction);

			CurrentLootBag = nullptr;
		}
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShooterCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AShooterCharacter::StopJump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::OnStartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::OnStopFiring);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::Reload);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AShooterCharacter::OnStartTargeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &AShooterCharacter::OnStopTargeting);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AShooterCharacter::OnStartRunning);
	PlayerInputComponent->BindAction("RunToggle", IE_Pressed, this, &AShooterCharacter::OnStartRunningToggle);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AShooterCharacter::OnStopRunning);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AShooterCharacter::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &AShooterCharacter::OnPrevWeapon);

	PlayerInputComponent->BindAction("ThrowItem", IE_Pressed, this, &AShooterCharacter::OnThrowItem);
}

void AShooterCharacter::OnStartTargeting()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		SetTargeting(true);
	//}
}

void AShooterCharacter::OnStopTargeting()
{
	SetTargeting(false);
}

void AShooterCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	//if (TargetingSound)
	//{
	//	UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
	//}

	if (Role < ROLE_Authority)
	{
		//ServerSetTargeting(bNewTargeting);
	}
}

void AShooterCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning, bToggle);
	}
}

bool AShooterCharacter::ServerSetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void AShooterCharacter::ServerSetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	SetRunning(bNewRunning, bToggle);
}

void AShooterCharacter::OnStartRunning()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (IsTargeting())
		{
			SetTargeting(false);
		}
		StopFiring();
		SetRunning(true, false);
	//}
}

void AShooterCharacter::OnStartRunningToggle()
{
	//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (IsTargeting())
		{
			SetTargeting(false);
		}
		StopFiring();
		SetRunning(true, true);
	//}
}

void AShooterCharacter::OnStopRunning()
{
	SetRunning(false, false);
}

float AShooterCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void AShooterCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
	}
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	SpawnDefaultInventory();
}

USkeletalMeshComponent* AShooterCharacter::GetPawnMesh() const
{
	return Mesh1PComp;
	//return IsFirstPerson() ? Mesh1P : GetMesh();
}

FVector AShooterCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	else
	{
		return Super::GetPawnViewLocation();
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AShooterCharacter, bDied);

	DOREPLIFETIME_CONDITION(AShooterCharacter, bWantsToRun, COND_SkipOwner);
}

float AShooterCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

float AShooterCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}
