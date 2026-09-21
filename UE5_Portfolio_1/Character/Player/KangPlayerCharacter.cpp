// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

// ────────── 커스텀 ──────────
#include "../../Weapon/WeaponBase.h"
#include "../../Weapon/RangedWeapon.h"
#include "../../Component/HUDComponent.h"
#include "../../Component/InteractionComponent.h"
#include "../../Component/InventoryComponent.h"
#include "../../Component/HealthComponent.h"
#include "../../Component/CombatComponent.h"
#include "../../Core/KangPlayerState.h"
#include "../../Core/KangPlayerController.h"
#include "../../Core/KangPlayerGameModeBase.h"
#include "../../Core/KangGameState.h"
#include "../../Core/UpgradeType.h"
#include "../../Data/UpgradeTable.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"


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

	// 힙파이어용 무기 거치대 — 카메라의 자식이라 항상 같은 화면 위치(오른쪽)에 무기가 보인다.
	// 실제 값은 BP_KangPlayer 에서 원하는 화면 배치에 맞춰 다시 조정할 placeholder.
	WeaponMount = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponMount"));
	WeaponMount->SetupAttachment(FollowCamera);
	WeaponMount->SetRelativeLocation(FVector(20.f, 15.f, -15.f));

	// 조준(ADS) 시 무기의 스코프 소켓이 정렬되는 목표 지점. 카메라 위치에 정확히 맞추면
	// 총열/몸체가 근접 클리핑 평면을 뚫고 들어가 화면에서 사라지므로, 카메라 앞쪽으로
	// 떨어뜨려 총이 항상 보이게 한다. BP_KangPlayer 에서 총과 조준선이 맞도록 위치/회전을
	// 눈으로 보며 조정할 것 (ScopeSocketRotationCorrection 과 함께 사용).
	AimMount = CreateDefaultSubobject<USceneComponent>(TEXT("AimMount"));
	AimMount->SetupAttachment(FollowCamera);
	AimMount->SetRelativeLocation(FVector(60.f, 0.f, -10.f));

	// 몸통 전신 애니메이션(AimOffset/Turn-in-Place/IK)을 없애는 대신 무기 메시만
	// 보여주는 방식으로 전환 — 캡슐 콜리전은 이동에 계속 쓰이므로 그대로 두고
	// 스켈레탈 메시만 숨긴다.
	GetMesh()->SetHiddenInGame(true, true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;
	// 몸통이 안 보이니 이동속도 기준으로 turn-in-place 를 흉내낼 필요가 없다 —
	// 항상 컨트롤러가 바라보는 방향으로 돈다.
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);
}

void AKangPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// BP 쪽 클래스 디폴트가 생성자 값을 조용히 덮어쓴 전례가 있어(이 프로젝트에서 여러 번
	// 겪음) BeginPlay 에서 한 번 더 확실히 굳힌다.
	GetMesh()->SetHiddenInGame(true, true);

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

	// 조준 시 시야를 좁혀 에임 지점에 집중된 느낌을 준다.
	const float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
	FollowCamera->SetFieldOfView(
		FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, DeltaTime, AimInterpSpeed)
	);

	// 힙파이어/조준 목표를 매 프레임 계산해서 무기를 그쪽으로 부드럽게 보간한다.
	// bIsAiming 과 무관하게 항상 호출 — 조준 진입/해제 양쪽 모두 스냅 없이 자연스럽게 전환된다.
	if (AWeaponBase* Weapon = GetActiveWeapon())
	{
		UpdateWeaponPose(Weapon, DeltaTime);
	}

	DrawDebugStatsOverlay();
}

void AKangPlayerCharacter::UpdateWeaponPose(AWeaponBase* Weapon, float DeltaTime) const
{
	USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
	FTransform TargetWorld;

	if (bIsAiming && WeaponMesh && WeaponMesh->DoesSocketExist(Weapon->ScopeSocketName))
	{
		// 무기 액터(=WeaponMesh, 루트 컴포넌트) 기준 스코프 소켓의 상대 트랜스폼.
		// RTS_Actor 를 쓰는 이유: WeaponMesh 가 무기 액터의 루트라서 "액터 기준"이 곧
		// SetActorTransform() 으로 우리가 세팅할 그 트랜스폼과 같은 좌표계가 된다.
		FTransform SocketRelativeToWeapon = WeaponMesh->GetSocketTransform(Weapon->ScopeSocketName, RTS_Actor);

		// 소켓/본 자체의 스케일은 우리 계산에 전혀 필요 없다 — 오히려 스켈레톤 쪽 스케일이
		// 비정상이면 아래 Inverse() 가 특이점에 걸려 위치·회전이 통째로 깨질 수 있으니
		// 항상 1로 고정해 원천 차단한다.
		SocketRelativeToWeapon.SetScale3D(FVector::OneVector);

		// 소켓 정면이 총구 반대(사수 쪽)를 보고 있을 때, 스켈레톤을 직접 안 고치고
		// 빠르게 뒤집어보기 위한 보정 — 소켓의 로컬 프레임 앞단에 끼워 넣는다.
		if (!Weapon->ScopeSocketRotationCorrection.IsNearlyZero())
		{
			SocketRelativeToWeapon = FTransform(Weapon->ScopeSocketRotationCorrection) * SocketRelativeToWeapon;
		}

		// "이 소켓이 AimMount(카메라 앞, 조준 시 정렬 목표)의 현재 위치/회전에 오려면
		// 무기 액터 자체가 어디 있어야 하는가"를 역산. 카메라 자체를 목표로 쓰지 않는 이유는
		// 총열/몸체가 근접 클리핑 평면에 파묻혀 화면에서 사라지기 때문 — AimMount 를 카메라
		// 앞쪽에 띄워두고 그 트랜스폼을 목표로 삼는다.
		TargetWorld = SocketRelativeToWeapon.Inverse() * AimMount->GetComponentTransform();
		TargetWorld.NormalizeRotation();

		if (TargetWorld.ContainsNaN())
		{
			// 계산이 깨졌다는 확실한 신호 — 무기를 이상한 곳으로 보내는 대신 이번 프레임은
			// 건너뛴다 (마지막으로 유효했던 위치에 그대로 머무름).
			UE_LOG(LogTemp, Error, TEXT("UpdateWeaponPose: %s 의 계산 결과에 NaN 포함 — ScopeSocket(%s)의 위치/회전을 확인할 것"),
				*Weapon->GetName(), *Weapon->ScopeSocketName.ToString());
			return;
		}
	}
	else
	{
		// 힙파이어 목표 — WeaponMount 기준 GripOffset (Equip() 이 처음 장착할 때와 동일한 기준).
		TargetWorld = Weapon->GripOffset * WeaponMount->GetComponentTransform();
	}

	// 목표 지점으로 순간이동하지 않도록 매 프레임 현재 트랜스폼에서 목표로 조금씩 보간한다.
	// 조준 진입/해제 양쪽 다 이 경로를 타므로 스냅 없이 대칭적으로 부드럽다.
	const FTransform Current = Weapon->GetActorTransform();
	const FVector NewLocation = FMath::VInterpTo(Current.GetLocation(), TargetWorld.GetLocation(), DeltaTime, AimInterpSpeed);
	const FQuat NewRotation = FQuat::Slerp(Current.GetRotation(), TargetWorld.GetRotation(), FMath::Clamp(DeltaTime * AimInterpSpeed, 0.f, 1.f));
	Weapon->SetActorTransform(FTransform(NewRotation, NewLocation));
}

void AKangPlayerCharacter::DrawDebugStatsOverlay() const
{
#if ENABLE_DRAW_DEBUG
	if (!bShowDebugStatsOverlay || !GEngine) return;

	const AKangPlayerState* PS = GetPlayerState<AKangPlayerState>();
	const AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;

	// ── 일차 / 국면 ──────────────────────────────────────────
	if (GS)
	{
		FString PhaseName;
		switch (GS->GetCurrentPhase())
		{
		case EGamePhase::Day:      PhaseName = TEXT("DAY (상점)");   break;
		case EGamePhase::Night:    PhaseName = TEXT("NIGHT (전투)"); break;
		case EGamePhase::GameOver: PhaseName = TEXT("GAME OVER");    break;
		default:                   PhaseName = TEXT("-");            break;
		}
		GEngine->AddOnScreenDebugMessage(9000, 0.f, FColor::Cyan, FString::Printf(
			TEXT("Day %d · %s · 스폰 대기 %d마리"),
			GS->GetDayNumber(), *PhaseName, GS->GetEnemiesRemainingToSpawn()));
	}

	// ── 체력 ─────────────────────────────────────────────────
	if (HealthComponent)
	{
		GEngine->AddOnScreenDebugMessage(9001, 0.f, FColor::Green, FString::Printf(
			TEXT("HP %.0f / %.0f   재생 %.2f/s"),
			HealthComponent->GetHealth(), HealthComponent->GetMaxHealth(), HealthComponent->GetRegenPerSecond()));
	}

	// ── 무기(실 데미지 / 연사속도) ───────────────────────────
	if (const ARangedWeapon* Weapon = Cast<ARangedWeapon>(GetActiveWeapon()))
	{
		const FGunData& Gun = Weapon->GetGunData();
		GEngine->AddOnScreenDebugMessage(9002, 0.f, FColor::Yellow, FString::Printf(
			TEXT("%s   DMG %.1f → %.1f (x%.2f)   간격 %.2fs (x%.2f)"),
			*Weapon->GetWeaponName().ToString(), Gun.Damage, Weapon->GetEffectiveDamage(),
			GetOutgoingDamageMultiplier(), Weapon->GetEffectiveFireInterval(), GetFireRateMultiplier()));
	}

	// ── 업그레이드 레벨 · 코인 ───────────────────────────────
	if (PS)
	{
		GEngine->AddOnScreenDebugMessage(9003, 0.f, FColor::Orange, FString::Printf(
			TEXT("업그레이드  공격력 Lv%d(x%.2f)  공격속도 Lv%d(x%.2f)  바리케이드체력 Lv%d(x%.2f)  체력재생 Lv%d  아군공격력 Lv%d(x%.2f)"),
			PS->GetUpgradeLevel(EUpgradeType::PlayerDamage), PS->GetUpgradeMultiplier(EUpgradeType::PlayerDamage),
			PS->GetUpgradeLevel(EUpgradeType::PlayerFireRate), PS->GetUpgradeMultiplier(EUpgradeType::PlayerFireRate),
			PS->GetUpgradeLevel(EUpgradeType::BarricadeHealth), PS->GetUpgradeMultiplier(EUpgradeType::BarricadeHealth),
			PS->GetUpgradeLevel(EUpgradeType::HealthRegen),
			PS->GetUpgradeLevel(EUpgradeType::AllyDamage), PS->GetUpgradeMultiplier(EUpgradeType::AllyDamage)));

		GEngine->AddOnScreenDebugMessage(9004, 0.f, FColor::White, FString::Printf(
			TEXT("코인 %d"), PS->GetCoin()));
	}
#endif
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
		EIC->BindAction(MenuAction, ETriggerEvent::Started, this, &AKangPlayerCharacter::ToggleMenu);
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
	if (bIsAiming)
	{
		StopAim(FInputActionValue());
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
	GetCharacterMovement()->DisableMovement();
	bIsFiring = false;
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
	if (bIsAiming)
	{
		StopAim(FInputActionValue());
	}
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
	if (bIsAiming) return;

	AWeaponBase* Weapon = GetActiveWeapon();
	if (!Weapon) return;

	USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
	if (!WeaponMesh || !WeaponMesh->DoesSocketExist(Weapon->ScopeSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("StartAim: %s has no socket '%s' — staying hip-fire"),
			*Weapon->GetName(), *Weapon->ScopeSocketName.ToString());
		return;
	}

	// 카메라는 전혀 안 건드린다 — 플래그만 올리면 Tick()의 UpdateWeaponPose() 가
	// 다음 프레임부터 무기를 AimMount 목표로 부드럽게 보간해간다.
	bIsAiming = true;
}

void AKangPlayerCharacter::StopAim(const FInputActionValue& Value)
{
	if (!bIsAiming) return;

	// 무기를 힙파이어 자리로 직접 되돌리지 않는다 — bIsAiming 을 내리면 Tick()의
	// UpdateWeaponPose() 가 다음 프레임부터 알아서 힙파이어 목표로 부드럽게 보간해간다.
	bIsAiming = false;
}

void AKangPlayerCharacter::Reload(const FInputActionValue& Value)
{
	CombatComponent->Reload();
}

void AKangPlayerCharacter::EquipWeapon1(const FInputActionValue& Value)
{
	if (bIsAiming) StopAim(FInputActionValue());
	InventoryComponent->EquipSlot(EWeaponSlot::Primary);
}

void AKangPlayerCharacter::EquipWeapon2(const FInputActionValue& Value)
{
	if (bIsAiming) StopAim(FInputActionValue());
	InventoryComponent->EquipSlot(EWeaponSlot::Secondary);
}

void AKangPlayerCharacter::EquipWeapon3(const FInputActionValue& Value)
{
	if (bIsAiming) StopAim(FInputActionValue());
	InventoryComponent->EquipSlot(EWeaponSlot::Throwable);
}

void AKangPlayerCharacter::EquipWeapon4(const FInputActionValue& Value)
{
	if (bIsAiming) StopAim(FInputActionValue());
}

void AKangPlayerCharacter::Escape(const FInputActionValue& Value)
{
	// 커맨드 메뉴가 열려있으면 그것부터 닫고, 아니면 일시정지를 토글한다.
	if (HUDComponent->IsMenuOpen())
	{
		HUDComponent->CloseMenu();
		return;
	}
	HUDComponent->TogglePause();
}

void AKangPlayerCharacter::ToggleMenu(const FInputActionValue& Value)
{
	HUDComponent->ToggleMenu();
}

