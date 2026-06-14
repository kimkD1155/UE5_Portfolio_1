// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
AKangPlayerCharacter::AKangPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AKangPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKangPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Equipped weapon: %s"), *EquippedWeapon->GetName());
	}*/

	UpdateInteractionTarget();
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
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
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

void AKangPlayerCharacter::Interact(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Interact action triggered!"));

	if (!CurrentInteractTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("No interactable target in sight."));
		return;
	}

	if (CurrentInteractTarget->Implements<UInteractableInterface>())
	{
		IInteractableInterface::Execute_Interact(CurrentInteractTarget, this);
	}
}

void AKangPlayerCharacter::Drop(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Drop action triggered!"));
	DropWeapon();
}

void AKangPlayerCharacter::StartFire(const FInputActionValue& Value)
{
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));
		return;
	}
	GetEquippedWeapon()->StartFire();
}

void AKangPlayerCharacter::StopFire(const FInputActionValue& Value)
{
	if (!EquippedWeapon) return;
	GetEquippedWeapon()->StopFire();
}

void AKangPlayerCharacter::StartAim(const FInputActionValue& Value)
{
	if (!EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon equipped"));
		return;
	}
	GetEquippedWeapon()->StartAim();
}

void AKangPlayerCharacter::StopAim(const FInputActionValue& Value)
{
	if (!EquippedWeapon) return;
	GetEquippedWeapon()->StopAim();
}


//-----------------여기까지 키 바인딩 및 입력 처리-----------------

void AKangPlayerCharacter::UpdateInteractionTarget()
{
	// 카메라 위치 / 방향 가져오기
	FVector CameraLocation;
	FRotator CameraRotation;
	GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + CameraRotation.Vector() * InteractTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, Params
	);

	// 맞은 게 있으면 초록색, 없으면 빨간색
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		bHit ? HitResult.ImpactPoint : TraceEnd,
		bHit ? FColor::Green : FColor::Red,
		false,    // 지속 표시 여부 (false = 매 프레임 갱신)
		-1.f,     // 지속 시간 (-1 = 1프레임만)
		0,
		1.5f      // 선 두께
	);

	AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;

	// IInteractable 구현한 액터만 대상으로
	if (HitActor && HitActor->Implements<UInteractableInterface>())
	{
		if (CurrentInteractTarget != HitActor)
		{
			CurrentInteractTarget = HitActor;
			// TODO: Step 4에서 HUD 힌트 텍스트 업데이트
		}
	}
	else
	{
		if (CurrentInteractTarget != nullptr)
		{
			CurrentInteractTarget = nullptr;
			// TODO: Step 4에서 HUD 힌트 숨기기
		}
	}
}


void AKangPlayerCharacter::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return;

	if (EquippedWeapon)
		DropWeapon();

	EquippedWeapon = Weapon;
	EquippedWeapon->Equip(this);
	CurrentInteractTarget = nullptr;
}



void AKangPlayerCharacter::DropWeapon()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->Unequip();

	FVector DropLocation = GetActorLocation()
		+ GetActorForwardVector() * 100.f
		+ FVector(0.f, 0.f, -50.f);
	EquippedWeapon->SetActorLocation(DropLocation);
	EquippedWeapon = nullptr;
}