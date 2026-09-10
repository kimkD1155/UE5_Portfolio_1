// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// ────────── 커스텀 ──────────
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Component/HUDComponent.h"
#include "../Component/InteractionComponent.h"
#include "../Component/InventoryComponent.h"
#include "../Component/HealthComponent.h"
#include "../Component/CombatComponent.h"
#include "../Core/KangPlayerState.h"
#include "../Core/KangPlayerController.h"
#include "../Core/KangPlayerGameModeBase.h"
#include "../Core/UpgradeType.h"
#include "../Data/UpgradeTable.h"
#include "../Props/Shop.h"


AKangPlayerCharacter::AKangPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	HUDComponent = CreateDefaultSubobject<UHUDComponent>(TEXT("HUDComponent"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

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

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);
}

void AKangPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	HealthComponent->OnDeath.AddDynamic(this, &AKangPlayerCharacter::HandleDeath);

	// 코인 델리게이트 바인딩
	if (AKangPlayerState* PS = GetPlayerState<AKangPlayerState>())
	{
		PS->OnCoinChanged.AddDynamic(HUDComponent, &UHUDComponent::UpdateCoinUI);
		HUDComponent->UpdateCoinUI(PS->GetCoin()); // 초기값 갱신

		PS->OnUpgradesChanged.AddDynamic(this, &AKangPlayerCharacter::HandleUpgradesChanged);
		HandleUpgradesChanged(); // 초기 반영
	}
}

float AKangPlayerCharacter::GetOutgoingDamageMultiplier() const
{
	if (const AKangPlayerState* PS = GetPlayerState<AKangPlayerState>())
	{
		return PS->GetUpgradeMultiplier(EUpgradeType::PlayerDamage);
	}
	return 1.f;
}

float AKangPlayerCharacter::GetFireRateMultiplier() const
{
	if (const AKangPlayerState* PS = GetPlayerState<AKangPlayerState>())
	{
		return PS->GetUpgradeMultiplier(EUpgradeType::PlayerFireRate);
	}
	return 1.f;
}

void AKangPlayerCharacter::HandleUpgradesChanged()
{
	const AKangPlayerState* PS = GetPlayerState<AKangPlayerState>();
	if (!PS || !HealthComponent) return;

	float RegenRate = 0.f;
	if (const UUpgradeTable* Table = PS->GetUpgradeTable())
	{
		RegenRate = PS->GetUpgradeLevel(EUpgradeType::HealthRegen)
			* Table->GetPerLevelValue(EUpgradeType::HealthRegen);
	}
	HealthComponent->SetRegenPerSecond(RegenRate);
}

void AKangPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMovementState();
	UpdateLeftHandIK();

	// 조준 FOV 보간만 매 프레임 필요. 탄약 UI 는 무기의 OnAmmoChanged 델리게이트가 담당.
	const float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
	FollowCamera->SetFieldOfView(
		FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed)
	);
}

void AKangPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKangPlayerCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKangPlayerCharacter::Look);
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
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
		EIC->BindAction(EscapeAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::Escape);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

AWeaponBase* AKangPlayerCharacter::GetActiveWeapon() const
{
	return InventoryComponent ? InventoryComponent->GetEquippedWeapon() : nullptr;
}

EWeaponType AKangPlayerCharacter::GetCurrentWeaponType() const
{
	if (AWeaponBase* Weapon = GetActiveWeapon())
	{
		return Weapon->GetWeaponType();
	}
	return EWeaponType::None;
}

void AKangPlayerCharacter::HandleDeath(AActor* /*DamageInstigator*/)
{
	// 플레이어 사망 = 게임오버. 국면 전환/UI 는 GameMode 가 처리한다.
	if (AKangPlayerGameModeBase* GM = GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>())
	{
		GM->NotifyPlayerDied();
	}

	CombatComponent->StopFire();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	GetCharacterMovement()->DisableMovement();
	bIsFiring = false;
	bIsAiming = false;
}

void AKangPlayerCharacter::UpdateMovementState()
{
	const float Speed = GetVelocity().Size();
	AimYaw = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, GetControlRotation().Yaw);

	if (Speed > MovingSpeedThreshold)
	{
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	else
	{
		if (AimYaw >= 90.f)
		{
			Turn = true;
			TurnDirection = ETurnDirection::Right;
		}
		else if (AimYaw <= -90.f)
		{
			Turn = true;
			TurnDirection = ETurnDirection::Left;
		}
		else
		{
			Turn = false;
			TurnDirection = ETurnDirection::None;
		}

		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}

void AKangPlayerCharacter::UpdateLeftHandIK()
{
	ARangedWeapon* EquippedWeapon = Cast<ARangedWeapon>(GetActiveWeapon());
	const bool bReloading = CombatComponent && CombatComponent->IsReloading();
	const bool bEquipping = InventoryComponent && InventoryComponent->GetIsEquipping();

	if (bEquipping || bReloading || !EquippedWeapon || !EquippedWeapon->GetWeaponMesh())
	{
		bShouldUseLeftHandIK = false;
		return;
	}

	if (EquippedWeapon->GetWeaponMesh()->DoesSocketExist(TEXT("LeftHandGrip")))
	{
		LeftHandIKTarget = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(TEXT("LeftHandGrip"), RTS_World);
		bShouldUseLeftHandIK = true;
	}
	else
	{
		bShouldUseLeftHandIK = false;
	}
}

void AKangPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0, ControlRotation.Yaw, 0); // Pitch/Roll 제거, Yaw만 사용

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X);
}

void AKangPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void AKangPlayerCharacter::OnJumped_Implementation()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AKangPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
}

void AKangPlayerCharacter::Interact(const FInputActionValue& Value)
{
	AActor* Target = InteractionComponent->GetCurrentInteractTarget();
	if (!Target) return;

	if (Target->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_Interact(Target, this);
	}
}

void AKangPlayerCharacter::Drop(const FInputActionValue& Value)
{
	InventoryComponent->DropWeapon();
}

void AKangPlayerCharacter::StartFire(const FInputActionValue& Value)
{
	if (InventoryComponent->GetIsEquipping()) return;

	bIsFiring = true;
	CombatComponent->StartFire();
}

void AKangPlayerCharacter::StopFire(const FInputActionValue& Value)
{
	CombatComponent->StopFire();
	bIsFiring = false;
}

void AKangPlayerCharacter::StartAim(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	if (!GetActiveWeapon()) return;

	bIsAiming = true;
	CombatComponent->StartAim();
}

void AKangPlayerCharacter::StopAim(const FInputActionValue& Value)
{
	if (!GetActiveWeapon()) return;

	bIsAiming = false;
	CombatComponent->StopAim();
}

void AKangPlayerCharacter::Reload(const FInputActionValue& Value)
{
	CombatComponent->Reload();
}

void AKangPlayerCharacter::EquipWeapon1(const FInputActionValue& Value)
{
	InventoryComponent->EquipSlot(EWeaponSlot::Primary);
}

void AKangPlayerCharacter::EquipWeapon2(const FInputActionValue& Value)
{
	InventoryComponent->EquipSlot(EWeaponSlot::Secondary);
}

void AKangPlayerCharacter::EquipWeapon3(const FInputActionValue& Value)
{
	InventoryComponent->EquipSlot(EWeaponSlot::Throwable);
}

void AKangPlayerCharacter::EquipWeapon4(const FInputActionValue& Value)
{
}

void AKangPlayerCharacter::Escape(const FInputActionValue& Value)
{
	// 열려있는 상점 찾아서 닫기
	TArray<AActor*> Shops;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShop::StaticClass(), Shops);
	for (AActor* Actor : Shops)
	{
		if (AShop* Shop = Cast<AShop>(Actor))
		{
			if (Shop->IsShopOpen())
			{
				Shop->CloseShop();
				return;
			}
		}
	}
}

