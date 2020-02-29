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
	WeaponAttachSocketName = "WeaponPoint";

	HealthComp = CreateDefaultSubobject<UShooterHealthComponent>(TEXT("HealthComp"));

	bIsTargeting = false;
	bWantsToRun = false;
	bWantsToFire = false;
	bWantsToRunToggled = false;

	TargetingSpeedModifier = 0.5f;
	RunningSpeedModifier = 1.5f;
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

	if (Role == ROLE_Authority)
	{
		// Spawn a default weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (!StarterWeaponClass)
		{
			return;
		}

		CurrentWeapon = GetWorld()->SpawnActor<AShooterWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwningPawn(this);
			CurrentWeapon->AttachToComponent(Mesh1PComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			CurrentWeapon->OnWeaponFired.AddDynamic(this, &AShooterCharacter::HandleOnWeaponFired);
		}
	}
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

//void AShooterCharacter::Fire()
//{
//	if (CurrentWeapon)
//	{
//		//CurrentWeapon->Fire();
//	}
//}

void AShooterCharacter::BeginFiring()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			//AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
			//if (MyPC && MyPC->IsGameInputAllowed())
			//{
			if (IsRunning())
			{
				SetRunning(false, false);
			}
			//	StartWeaponFire();
			//}

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
		CurrentWeapon->Reload();
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

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWantsToRunToggled && !IsRunning())
	{
		SetRunning(false, false);
	}

	//float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	//
	//float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomedInterpSpeed);
	//
	//CameraComp->SetFieldOfView(NewFOV);
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

	//PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &AShooterCharacter::BeginZoom);
	//PlayerInputComponent->BindAction("Zoom", IE_Released, this, &AShooterCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::BeginFiring);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::StopFiring);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::Reload);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AShooterCharacter::OnStartTargeting);
	PlayerInputComponent->BindAction("Targeting", IE_Released, this, &AShooterCharacter::OnStopTargeting);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AShooterCharacter::OnStartRunning);
	PlayerInputComponent->BindAction("RunToggle", IE_Pressed, this, &AShooterCharacter::OnStartRunningToggle);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AShooterCharacter::OnStopRunning);
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
		//ServerSetRunning(bNewRunning, bToggle);
	}
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
}

float AShooterCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

float AShooterCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}
