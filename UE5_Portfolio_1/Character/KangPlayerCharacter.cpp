// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Component/HUDComponent.h"
#include "../Component/InteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AKangPlayerCharacter::AKangPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HUDComponent = CreateDefaultSubobject<UHUDComponent>(TEXT("HUDComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));

	// ── 카메라 ────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->SocketOffset = FVector(0.f, 80.f, 80.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = DefaultFOV;
}

// Called when the game starts or when spawned
void AKangPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//GetMesh()->SetOwnerNoSee(true);   // 1인칭 시점에서 메쉬 안 보이게(임시)

}

// Called every frame
void AKangPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(InteractionComponent->GetEquippedWeapon()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Test"))
		HUDComponent->UpdateAmmoUI(
			Ranged->GetCurrentAmmo(),
			Ranged->GetReserveAmmo(),
			Ranged->GetWeaponName()
		);

		float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
		FollowCamera->SetFieldOfView(
			FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed)
		);
	}

}

// Called to bind functionality to input
void AKangPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKangPlayerCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKangPlayerCharacter::Look);
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EIC->BindAction(RunAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::StartRunning);
		EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &AKangPlayerCharacter::StopRunning);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::Interact);
		EIC->BindAction(DropAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::Drop);
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::StartFire);
		EIC->BindAction(FireAction, ETriggerEvent::Completed, this, &AKangPlayerCharacter::StopFire);
		EIC->BindAction(AimAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::StartAim);
		EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &AKangPlayerCharacter::StopAim);
		EIC->BindAction(ReloadAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::Reload);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AKangPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		//// add movement 
		//AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		//AddMovementInput(GetActorRightVector(), MovementVector.X);

		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, MovementVector.Y);
		AddMovementInput(Right, MovementVector.X);
	}
}

void AKangPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}

}

void AKangPlayerCharacter::StartRunning(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	ExitCombatMode();
}

void AKangPlayerCharacter::StopRunning(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AKangPlayerCharacter::Interact(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact action triggered!"));

	if (!InteractionComponent->GetCurrentInteractTarget())
	{
		UE_LOG(LogTemp, Warning, TEXT("No interactable target in sight."));
		return;
	}


	if (InteractionComponent->GetCurrentInteractTarget()->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_Interact(InteractionComponent->GetCurrentInteractTarget(), this);
	}
}

void AKangPlayerCharacter::Drop(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Drop action triggered!"));
	InteractionComponent->DropWeapon();
}

void AKangPlayerCharacter::StartFire(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (!InteractionComponent->GetEquippedWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));

		return;
	}
	EnterCombatMode();
	InteractionComponent->GetEquippedWeapon()->StartFire();
}

void AKangPlayerCharacter::StopFire(const FInputActionValue& Value)
{
	if (!InteractionComponent->GetEquippedWeapon()) return;
	
	
	InteractionComponent->GetEquippedWeapon()->StopFire();
}

void AKangPlayerCharacter::StartAim(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (!InteractionComponent->GetEquippedWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));
		return;
	}
	bIsAiming = true;
	EnterCombatMode();
	InteractionComponent->GetEquippedWeapon()->StartAim();
}

void AKangPlayerCharacter::StopAim(const FInputActionValue& Value)
{
	if (!InteractionComponent->GetEquippedWeapon()) return;
	
	bIsAiming = false;
	InteractionComponent->GetEquippedWeapon()->StopAim();
}

void AKangPlayerCharacter::Reload(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("AKangPlayerCharacter::Reload"))
	if (!InteractionComponent->GetEquippedWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));
		return;
	}

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(InteractionComponent->GetEquippedWeapon());

	if (RangedWeapon)
		RangedWeapon->Reload();
}


//-----------------여기까지 키 바인딩 및 입력 처리-----------------

EWeaponType AKangPlayerCharacter::GetCurrentWeaponType() const
{
	if (InteractionComponent->GetEquippedWeapon())
	{
		return InteractionComponent->GetEquippedWeapon()->GetWeaponType();
	}
	return EWeaponType::None;
}

void AKangPlayerCharacter::EnterCombatMode()
{
	bIsInCombatMode = true;
	bUseControllerRotationYaw = true; // 무기 발사 시 캐릭터 회전 활성화
	GetCharacterMovement()->bOrientRotationToMovement = false; // 무기 발사 시 캐릭터 이동 방향 회전 비활성화

	// 타이머 리셋 (이미 있으면 새로 갱신)
	GetWorldTimerManager().SetTimer(
		CombatModeTimerHandle,
		this,
		&AKangPlayerCharacter::ExitCombatMode,
		CombatModeTimeout,
		false
	);
}

void AKangPlayerCharacter::ExitCombatMode()
{
	bIsInCombatMode = false;
	bUseControllerRotationYaw = false; // 무기 발사 종료 시 캐릭터 회전 비활성화
	GetCharacterMovement()->bOrientRotationToMovement = true; // 무기 발사 종료 시 캐릭터 이동 방향 회전 활성화
}