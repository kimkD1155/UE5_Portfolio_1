// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

//ㅡㅡㅡㅡㅡㅡ 커스텀 ㅡㅡㅡㅡㅡㅡ
#include "../Weapon/WeaponBase.h"
#include "../Weapon/RangedWeapon.h"
#include "../Component/HUDComponent.h"
#include "../Component/InteractionComponent.h"
#include "../Component/InventoryComponent.h"
#include "../Animation/KangAnimInstance.h"
#include "../Core/KangPlayerState.h"
#include "../Core/KangPlayerController.h"
#include "../Props/Shop.h"


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

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);
}

// Called when the game starts or when spawned
void AKangPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	// 코인 델리게이트 바인딩
	if (AKangPlayerState* PS = GetPlayerState<AKangPlayerState>())
	{
		PS->OnCoinChanged.AddDynamic(HUDComponent, &UHUDComponent::UpdateCoinUI);
		HUDComponent->UpdateCoinUI(PS->GetCoin()); // 초기값 갱신
	}

	if (UKangAnimInstance* AnimInst = Cast<UKangAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->OnReloadFinishedDelegate.AddDynamic(this, &AKangPlayerCharacter::OnReloadNotify);
	}
}

// Called every frame
void AKangPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMovementState();
	UpdateLeftHandIK();
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
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AKangPlayerCharacter::UpdateMovementState()
{
	float Speed = GetVelocity().Size();
	AimYaw = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, GetControlRotation().Yaw);

	if (Speed > 10.f)
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
	ARangedWeapon* EquippedWeapon = Cast<ARangedWeapon>(InventoryComponent->GetEquippedWeapon());
	if (bIsEquipping || !EquippedWeapon || !EquippedWeapon->GetWeaponMesh())
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
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0, ControlRotation.Yaw, 0); // Pitch/Roll 제거, Yaw만 사용

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MovementVector.Y);
	AddMovementInput(Right, MovementVector.X);
	
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
	
}

void AKangPlayerCharacter::StartFire(const FInputActionValue& Value)
{
	AKangPlayerController* PC = Cast<AKangPlayerController>(GetController());
	if (PC && PC->IsInPlacementMode())
	{
		PC->ConfirmPlacement(); // 배치 모드면 설치
		return;

	}

	ARangedWeapon* RangedWeapon = Cast<ARangedWeapon>(InventoryComponent->GetEquippedWeapon());

	if (!RangedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped1"));
		return;
	}
	if (bIsEquipping == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire while equipping a weapon."));
		return;
	}

	bIsFiring = true;
	

	

	if (RangedWeapon->GetGunState() == EGunState::Reloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reloading..."));
		return;
	}

	RangedWeapon->StartFire();

	

	switch (RangedWeapon->GetWeaponType())
	{
		case EWeaponType::Pistol:
			PlayAnimMontage(PistolFireMontage);
			break;
		case EWeaponType::Rifle:
			PlayAnimMontage(RifleFireMontage);
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Fire not implemented for this weapon type."));
			return;

	}
	
}

void AKangPlayerCharacter::StopFire(const FInputActionValue& Value)
{
	if (!InventoryComponent->GetEquippedWeapon()) return;
	
	
	InventoryComponent->GetEquippedWeapon()->StopFire();
	bIsFiring = false;
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

	if (!RangedWeapon->CanReload())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload: No ammo or already full."));
		return;
	}

	// 몽타주 있는지 확인
	if (!PistolReloadMontage || !RifleReloadMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Reload montages not set."));
		return;
	}

	RangedWeapon->Reload();
	ReloadingWeapon = RangedWeapon;

	switch (RangedWeapon->GetWeaponType())
	{
		case EWeaponType::Pistol:
			PlayReloadMontage(PistolReloadMontage);
			break;
		case EWeaponType::Rifle:
			PlayReloadMontage(RifleReloadMontage);
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Reload not implemented for this weapon type."));
			return;
	}
	
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
	UE_LOG(LogTemp, Warning, TEXT("Equip Weapon 4 action triggered!"));
}

void AKangPlayerCharacter::Escape(const FInputActionValue& Value)
{
	AKangPlayerController* PC = Cast<AKangPlayerController>(GetController());
	if (PC && PC->IsInPlacementMode())
	{
		PC->CancelPlacement();
		return;
	}

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


//-----------------여기까지 키 바인딩 및 입력 처리-----------------

EWeaponType AKangPlayerCharacter::GetCurrentWeaponType() const
{
	if (InventoryComponent->GetEquippedWeapon())
	{
		return InventoryComponent->GetEquippedWeapon()->GetWeaponType();
	}
	return EWeaponType::None;
}


void AKangPlayerCharacter::OnReloadNotify()
{
	
	if (ReloadingWeapon)
	{
		ReloadingWeapon->ReloadFinished();
	}
	
}



void AKangPlayerCharacter::PlayEquipMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;

	bIsEquipping = true;
	PlayAnimMontage(MontageToPlay);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AKangPlayerCharacter::OnEquipMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}
}

void AKangPlayerCharacter::PlayReloadMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;

	PlayAnimMontage(MontageToPlay);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AKangPlayerCharacter::OnReloadMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}
}


void AKangPlayerCharacter::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ReloadingWeapon)
	{
		ReloadingWeapon->SetGunState(bInterrupted ? EGunState::Idle : EGunState::Idle);
		ReloadingWeapon = nullptr;
	}
}

void AKangPlayerCharacter::OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsEquipping = false;
}

