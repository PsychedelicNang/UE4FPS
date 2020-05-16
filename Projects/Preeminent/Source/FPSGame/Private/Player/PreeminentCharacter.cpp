// Fill out your copyright notice in the Description page of Project Settings.


#include "PreeminentCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/PreeminentWeapon.h"
#include "Components/PreeminentHealthComponent.h"
#include "Preeminent.h"
#include "Animation/AnimInstance.h"
#include "PreeminentCharacterMovement.h"
#include "Interactables/PreeminentLootBag.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "PreeminentStockpile.h"
#include "TimerManager.h"
#include "PreeminentGameMode.h"

APreeminentCharacter::APreeminentCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPreeminentCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraComponent	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComp->SetupAttachment(GetCapsuleComponent());
	CameraComp->RelativeLocation = FVector(0, 0, BaseEyeHeight); // Position the camera
	CameraComp->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComp->SetupAttachment(CameraComp);
	Mesh1PComp->CastShadow = true;
	Mesh1PComp->bOnlyOwnerSee = true;
	Mesh1PComp->bOwnerNoSee = false;
	Mesh1PComp->bCastDynamicShadow = false;
	Mesh1PComp->bReceivesDecals = false;
	Mesh1PComp->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1PComp->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh1PComp->RelativeRotation = FRotator(2.0f, -15.0f, 5.0f);
	Mesh1PComp->RelativeLocation = FVector(0, 0, -155.0f);

	//*Correct location for ShooterGame Character Assets!**//
	//Mesh1PComp->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
	//Mesh1PComp->RelativeLocation = FVector(0, 0, -150.0f);
	//***//

	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	//GetMesh()->CastShadow = true;
	//GetMesh()->bCastDynamicShadow = true;
	//GetMesh()->bCastHiddenShadow = true;
	//GetMesh()->bAffectDistanceFieldLighting = true; // ?
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	//GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 65.0f;
	ZoomedInterpSpeed = 20.0f;
	DefaultFOV = 90.0f;

	HealthComp = CreateDefaultSubobject<UPreeminentHealthComponent>(TEXT("HealthComp"));

	bIsTargeting = false;
	bWantsToRun = false;
	bWantsToFire = false;
	bWantsToRunToggled = false;
	bIsCarryingLootBag = false;

	TargetingSpeedModifier = 0.5f;
	RunningSpeedModifier = 1.5f;
	InteractableDistance = 100.0f;

	LaunchStrength = 1000.0f;

	CurrentLootBag = nullptr;

	WeaponAttachPointName = "WeaponSocket";
	LootBagAttachPointName = "LootBagPoint";
}

void APreeminentCharacter::BeginPlay()
{
	Super::BeginPlay();

	//DefaultFOV = CameraComp->FieldOfView;

	if (HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &APreeminentCharacter::OnHealthChanged);
	}

	SpawnDefaultInventory();
}


void APreeminentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//bIsTargeting = true;

	if (!IsLocallyControlled())
	{
		FRotator NewRot = CameraComp->RelativeRotation;
		NewRot.Pitch = RemoteViewPitch * 360.0f / 255.0f; // Uncompress RemoteViewPitch

		CameraComp->SetRelativeRotation(NewRot);
	}

	if (bWantsToRunToggled && !IsRunning())
	{
		SetRunning(false, false);
	}

	if (CameraComp)
	{
		float TargetFOV = bIsTargeting ? ZoomedFOV : DefaultFOV;

		// Lerp to our desired FOV
		float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, CurrentWeapon ? CurrentWeapon->WeaponConfig.AimDownSightTime * 100 : ZoomedInterpSpeed);

		CameraComp->SetFieldOfView(NewFOV);
	}

	if (bPendingRequestLootBag == false && !bIsCarryingLootBag && CurrentLootBag == nullptr)
	{
		if (!bIsAttemptingInteract)
		{
			// Don't do anything if the user is not attemping to iteract with something
			return;
		}

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
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
			InteractHeldTime = PlayerController->GetInputKeyTimeDown(FKey("F"));
			if (InteractHeldTime > 1.0f)
			{
				APreeminentLootBag* LootBag = Cast<APreeminentLootBag>(Hit.Actor);

				if (LootBag)
				{
					EquipLootBag(LootBag);
				}

				APreeminentStockpile* Stockpile = Cast<APreeminentStockpile>(Hit.Actor);

				if (Stockpile)
				{
					bPendingRequestLootBag = true;
					GetLootBagFromStockpile(Stockpile, this);
				}
			}
		}
	}
}

void APreeminentCharacter::GetLootBagFromStockpile(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester)
{
	if (Requester)
	{
		if (Role == ROLE_Authority)
		{
			if (Stockpile)
			{
				Stockpile->GetLootBagFromPile(Requester);
			}
		}
		else
		{
			ServerRequestLootBag(Stockpile, Requester);
		}
	}
}

bool APreeminentCharacter::RequestRestartDeadPlayers()
{
	if (Role == ROLE_Authority)
	{
		APreeminentGameMode* GameMode = Cast<APreeminentGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			return GameMode->RequestRestartDeadPlayers();
		}
	}
	else
	{
		ServerRequestRestartDeadPlayers();
		return true; // Not the correct way to do this, but this function is only for development.. 
	}

	return false;
}

bool APreeminentCharacter::ServerRequestRestartDeadPlayers_Validate()
{
	return true;
}

void APreeminentCharacter::ServerRequestRestartDeadPlayers_Implementation()
{
	RequestRestartDeadPlayers();
}

bool APreeminentCharacter::ServerRequestLootBag_Validate(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester)
{
	return true;
}

void APreeminentCharacter::ServerRequestLootBag_Implementation(APreeminentStockpile* Stockpile, APreeminentCharacter* Requester)
{
	GetLootBagFromStockpile(Stockpile, Requester);
}

void APreeminentCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void APreeminentCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void APreeminentCharacter::BeginCrouch()
{
	Crouch();
}

void APreeminentCharacter::EndCrouch()
{
	UnCrouch();
}

void APreeminentCharacter::StartJump()
{
	Jump();
}

void APreeminentCharacter::StopJump()
{
	StopJumping();
}

void APreeminentCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void APreeminentCharacter::EndZoom()
{
	bWantsToZoom = false;
}

void APreeminentCharacter::OnStartFiring()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{

	if (IsRunning())
	{
		SetRunning(false, false);
	}
	BeginFiring();

	//}
}

void APreeminentCharacter::OnStopFiring()
{
	StopFiring();
}

void APreeminentCharacter::BeginFiring()
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

void APreeminentCharacter::StopFiring()
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

bool APreeminentCharacter::IsFiring() const
{
	return bWantsToFire;
}

bool APreeminentCharacter::IsCarryingLootBag() const
{
	return bIsCarryingLootBag;
}

APreeminentWeapon * APreeminentCharacter::GetWeapon() const
{
	return CurrentWeapon;
}

bool APreeminentCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return (bWantsToRun || bWantsToRunToggled) && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorForwardVector()) > -0.1;
}

bool APreeminentCharacter::IsTargeting() const
{
	return bIsTargeting;
}

void APreeminentCharacter::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
	}
}

void APreeminentCharacter::StartInteract()
{
	bIsAttemptingInteract = true;
	InteractHeldTime = 0.0f;
}

void APreeminentCharacter::EndInteract()
{
	bIsAttemptingInteract = false;
	InteractHeldTime = 0.0f;
}

void APreeminentCharacter::OnHealthChanged(UPreeminentHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
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

void APreeminentCharacter::HandleOnWeaponFired(APreeminentWeapon * WeaponFired)
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

void APreeminentCharacter::SpawnDefaultInventory()
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
			APreeminentWeapon* NewWeapon = GetWorld()->SpawnActor<APreeminentWeapon>(DefaultInventoryClasses[i], SpawnInfo);
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
}

void APreeminentCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	// remove all weapons from inventory and destroy them
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		APreeminentWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}

void APreeminentCharacter::AddWeapon(APreeminentWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
	}
}

void APreeminentCharacter::RemoveWeapon(APreeminentWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
	}
}

APreeminentWeapon* APreeminentCharacter::FindWeapon(TSubclassOf<APreeminentWeapon> WeaponClass)
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

void APreeminentCharacter::EquipWeapon(APreeminentWeapon* Weapon)
{
	if (Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon, CurrentWeapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

bool APreeminentCharacter::ServerEquipWeapon_Validate(APreeminentWeapon* Weapon)
{
	return true;
}

void APreeminentCharacter::ServerEquipWeapon_Implementation(APreeminentWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

bool APreeminentCharacter::ServerEquipLootBag_Validate(APreeminentLootBag* LootBag)
{
	return true;
}

void APreeminentCharacter::ServerEquipLootBag_Implementation(APreeminentLootBag* LootBag)
{
	EquipLootBag(LootBag);
}

void APreeminentCharacter::OnRep_CurrentLootBag(APreeminentLootBag * LootBag)
{
	SetCurrentLootBag(CurrentLootBag);
}

void APreeminentCharacter::EquipLootBag(APreeminentLootBag* LootBag)
{
	if (LootBag)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentLootBag(LootBag);
		}
		else
		{
			ServerEquipLootBag(LootBag);
		}
	}
}

void APreeminentCharacter::SetCurrentLootBag(APreeminentLootBag* LootBag)
{
	if (LootBag)
	{
		LootBag->OnEquip();
	}

	CurrentLootBag = LootBag;

	// equip new one
	if (CurrentLootBag)
	{
		CurrentLootBag->SetOwningPawn(this);	// Make sure LootBag's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentLootBag will rep after APreeminentootBag::MyPawn!

		CurrentLootBag->OnEquip();
	}

	bIsCarryingLootBag = true;
}

USkeletalMeshComponent* APreeminentCharacter::GetSpecifcPawnMesh(bool WantFirstPerson) const
{
	return WantFirstPerson == true ? Mesh1PComp : GetMesh();
}

float APreeminentCharacter::GetCarryingLootBagSpeedModifier() const
{
	return CurrentLootBag ? CurrentLootBag->GetCarryingLootBagSpeedModifier() : 1.0f;
}

FName APreeminentCharacter::GetWeaponAttachPointName() const
{
	return WeaponAttachPointName;
}

FName APreeminentCharacter::GetLootBagAttachPointName() const
{
	return LootBagAttachPointName;
}

void APreeminentCharacter::SetCurrentWeapon(APreeminentWeapon* NewWeapon, APreeminentWeapon* LastWeapon)
{
	APreeminentWeapon* LocalLastWeapon = nullptr;

	if (LastWeapon != nullptr)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!

		NewWeapon->OnEquip(LastWeapon);
	}
}

void APreeminentCharacter::OnNextWeapon()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			APreeminentWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon);
		}
	//}
}

void APreeminentCharacter::OnPrevWeapon()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			APreeminentWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
			EquipWeapon(PrevWeapon);
		}
	//}
}

void APreeminentCharacter::MulticastOnThrowItem_Implementation(FVector CamLoc, FRotator CamRot)
{
	if (bIsCarryingLootBag && CurrentLootBag)
	{
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

	bPendingRequestLootBag = false;
}

void APreeminentCharacter::OnThrowItem(FVector CamLoc, FRotator CamRot)
{
	// If we are the server, tell ourself and all clients to throw the object. We need to do it this way because if we handle it in OnRep_CurrentLootBag, things can get messy.
	if (Role == ROLE_Authority)
	{
		MulticastOnThrowItem(CamLoc, CamRot);
	}
	// If we are a client, tell the server we want to throw the object. This will then cause the server to call the multicast which tells everyone to throw the object.
	else
	{
		ServerThrowItem(CamLoc, CamRot);
	}

	bPendingRequestLootBag = false;
}

void APreeminentCharacter::OnThrowPressed()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		// Get the location and rotation of the camera
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		OnThrowItem(CamLoc, CamRot);
	}
}

bool APreeminentCharacter::ServerThrowItem_Validate(FVector CamLoc, FRotator CamRot)
{
	return true;
}

void APreeminentCharacter::ServerThrowItem_Implementation(FVector CamLoc, FRotator CamRot)
{
	OnThrowItem(CamLoc, CamRot);
}

// Called to bind functionality to input
void APreeminentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APreeminentCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APreeminentCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &APreeminentCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APreeminentCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APreeminentCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &APreeminentCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APreeminentCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APreeminentCharacter::StopJump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APreeminentCharacter::OnStartFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APreeminentCharacter::OnStopFiring);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APreeminentCharacter::Reload);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &APreeminentCharacter::OnStartTargeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &APreeminentCharacter::OnStopTargeting);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &APreeminentCharacter::OnStartRunning);
	PlayerInputComponent->BindAction("RunToggle", IE_Pressed, this, &APreeminentCharacter::OnStartRunningToggle);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &APreeminentCharacter::OnStopRunning);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &APreeminentCharacter::OnNextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &APreeminentCharacter::OnPrevWeapon);

	PlayerInputComponent->BindAction("ThrowItem", IE_Pressed, this, &APreeminentCharacter::OnThrowPressed);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APreeminentCharacter::StartInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APreeminentCharacter::EndInteract);
}

void APreeminentCharacter::OnStartTargeting()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
	//if (MyPC && MyPC->IsGameInputAllowed())
	//{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		SetTargeting(true);
	//}
}

void APreeminentCharacter::OnStopTargeting()
{
	SetTargeting(false);
}

void APreeminentCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	//if (TargetingSound)
	//{
	//	UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
	//}

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(bNewTargeting);
	}
}

bool APreeminentCharacter::ServerSetTargeting_Validate(bool bNewTargeting)
{
	return true;
}

void APreeminentCharacter::ServerSetTargeting_Implementation(bool bNewTargeting)
{
	SetTargeting(bNewTargeting);
}

void APreeminentCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning, bToggle);
	}
}

bool APreeminentCharacter::ServerSetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void APreeminentCharacter::ServerSetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	SetRunning(bNewRunning, bToggle);
}

void APreeminentCharacter::OnStartRunning()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
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

void APreeminentCharacter::OnStartRunningToggle()
{
	//APreeminentPlayerController* MyPC = Cast<APreeminentPlayerController>(Controller);
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

void APreeminentCharacter::OnStopRunning()
{
	SetRunning(false, false);
}

float APreeminentCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void APreeminentCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
	}
}

bool APreeminentCharacter::IsFirstPerson() const
{
	return /*IsAlive() &&*/ Controller && Controller->IsLocalPlayerController();
}

USkeletalMeshComponent* APreeminentCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? Mesh1PComp : GetMesh();
}

FVector APreeminentCharacter::GetPawnViewLocation() const
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

void APreeminentCharacter::OnRep_CurrentWeapon(APreeminentWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void APreeminentCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(APreeminentCharacter, Inventory, COND_OwnerOnly);

	DOREPLIFETIME(APreeminentCharacter, CurrentWeapon);
	DOREPLIFETIME(APreeminentCharacter, bDied);
	DOREPLIFETIME(APreeminentCharacter, CurrentLootBag);

	DOREPLIFETIME_CONDITION(APreeminentCharacter, bIsTargeting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APreeminentCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APreeminentCharacter, bIsCarryingLootBag, COND_SkipOwner);
}

float APreeminentCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

float APreeminentCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}

FRotator APreeminentCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

