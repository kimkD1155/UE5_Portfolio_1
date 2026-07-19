// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"

//ㅡㅡㅡㅡㅡㅡ 커스텀 컴포넌트 ㅡㅡㅡㅡㅡㅡ
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Component/HUDComponent.h"
#include "../Component/InteractionComponent.h"
#include "../Component/InventoryComponent.h"
#include "../Animation/KangAnimInstance.h"

//ㅡㅡㅡㅡㅡㅡ 커스텀 파일 ㅡㅡㅡㅡㅡㅡ
#include "../Core/KangPlayerState.h"


// Sets default values
AKangPlayerCharacter::AKangPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HUDComponent = CreateDefaultSubobject<UHUDComponent>(TEXT("HUDComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	// ── 카메라 ────────────────────────────────────
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->SocketOffset = FVector(0.f, 80.f, 60.f);
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

	// 코인 델리게이트 바인딩
	if (AKangPlayerState* PS = GetPlayerState<AKangPlayerState>())
	{
		PS->OnCoinChanged.AddDynamic(HUDComponent, &UHUDComponent::UpdateCoinUI);
		HUDComponent->UpdateCoinUI(PS->GetCoin()); // 초기값 갱신
	}

	if (UKangAnimInstance* AnimInst = Cast<UKangAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->OnReloadFinishedDelegate.AddDynamic(this, &AKangPlayerCharacter::OnReloadNotify);
		AnimInst->OnMontageEnded.AddDynamic(this, &AKangPlayerCharacter::OnReloadMontageEnded);
	}
}

// Called every frame
void AKangPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ARangedWeapon* Ranged = Cast<ARangedWeapon>(InventoryComponent->GetEquippedWeapon()))
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
		EIC->BindAction(Num1Action, ETriggerEvent::Started, this, &AKangPlayerCharacter::EquipWeapon1);
		EIC->BindAction(Num2Action, ETriggerEvent::Started, this, &AKangPlayerCharacter::EquipWeapon2);
		EIC->BindAction(Num3Action, ETriggerEvent::Started, this, &AKangPlayerCharacter::EquipWeapon3);
		EIC->BindAction(Num4Action, ETriggerEvent::Started, this, &AKangPlayerCharacter::EquipWeapon4);
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
	
	InventoryComponent->DropWeapon();
	ExitCombatMode();
}

void AKangPlayerCharacter::StartFire(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (!InventoryComponent->GetEquippedWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));

		return;
	}
	EnterCombatMode();
	InventoryComponent->GetEquippedWeapon()->StartFire();
}

void AKangPlayerCharacter::StopFire(const FInputActionValue& Value)
{
	if (!InventoryComponent->GetEquippedWeapon()) return;
	
	
	InventoryComponent->GetEquippedWeapon()->StopFire();
}

void AKangPlayerCharacter::StartAim(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (!InventoryComponent->GetEquippedWeapon())
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));
		return;
	}
	bIsAiming = true;
	EnterCombatMode();
	InventoryComponent->GetEquippedWeapon()->StartAim();
}

void AKangPlayerCharacter::StopAim(const FInputActionValue& Value)
{
	if (!InventoryComponent->GetEquippedWeapon()) return;
	
	bIsAiming = false;
	InventoryComponent->GetEquippedWeapon()->StopAim();
}

void AKangPlayerCharacter::Reload(const FInputActionValue& Value)
{

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(InventoryComponent->GetEquippedWeapon());

	if (!RangedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped1"));
		return;
	}

	if (!RifleReloadMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped2"));
		return;
	}

	if (!RangedWeapon->CanReload())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload: No ammo or already full."));
		return;
	}

	ReloadingWeapon = RangedWeapon;
	RangedWeapon->Reload();
	PlayAnimMontage(RifleReloadMontage);

}



void AKangPlayerCharacter::EquipWeapon1(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Equip Weapon 1 action triggered!"));
}

void AKangPlayerCharacter::EquipWeapon2(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Equip Weapon 2 action triggered!"));
}

void AKangPlayerCharacter::EquipWeapon3(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Equip Weapon 3 action triggered!"));
}

void AKangPlayerCharacter::EquipWeapon4(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Equip Weapon 4 action triggered!"));
}


//-----------------여기까지 키 바인딩 및 입력 처리-----------------

EWeaponType AKangPlayerCharacter::GetCurrentWeaponType() const
{
	if (InventoryComponent->GetEquippedWeapon())
	{
		return InventoryComponent->GetEquippedWeapon()->GetWeaponType();
	}
	return EWeaponType::None;
}

void AKangPlayerCharacter::EnterCombatMode()
{
	if (InventoryComponent->GetEquippedWeapon() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped. Cannot enter combat mode."));
		return;
	}
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

void AKangPlayerCharacter::OnReloadNotify()
{
	if (ReloadingWeapon)
	{
		ReloadingWeapon->ReloadFinished();
		
	}
	
}

// 몽타주 중단 시 상태 복구
void AKangPlayerCharacter::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != RifleReloadMontage) return;

	if (ReloadingWeapon)
	{
		ReloadingWeapon->SetGunState(bInterrupted ? EGunState::Idle : EGunState::Idle);
		ReloadingWeapon = nullptr;
	}
}