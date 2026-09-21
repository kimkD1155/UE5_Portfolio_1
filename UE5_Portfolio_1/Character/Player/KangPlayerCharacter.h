// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Weapon/WeaponBase.h"          // EWeaponType
#include "../../Interface/WeaponHolder.h"
#include "KangPlayerCharacter.generated.h"

// ─────────── 전방 선언 ───────────
class UHUDComponent;
class UInteractionComponent;
class UInventoryComponent;
class UHealthComponent;
class UCombatComponent;

class ARangedWeapon;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
struct FInputActionValue;

UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerCharacter : public ACharacter, public IWeaponHolder
{
	GENERATED_BODY()

public:
	AKangPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IWeaponHolder — 현재 장착 무기는 인벤토리 컴포넌트가 관리
	virtual AWeaponBase* GetActiveWeapon() const override;
	virtual float GetOutgoingDamageMultiplier() const override;
	virtual float GetFireRateMultiplier() const override;

	//────────────────────────── 키 입력 관련 ──────────────────────────
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DropAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Num1Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Num2Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Num3Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Num4Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* EscapeAction;
	// B 키 — 커맨드 메뉴(구매/수색/동료 무기 교체) 토글. 구 AShop/AScavengePoint 상호작용을 대체.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MenuAction;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Drop(const FInputActionValue& Value);
	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
	void StartAim(const FInputActionValue& Value);
	void StopAim(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);
	void EquipWeapon1(const FInputActionValue& Value);
	void EquipWeapon2(const FInputActionValue& Value);
	void EquipWeapon3(const FInputActionValue& Value);
	void EquipWeapon4(const FInputActionValue& Value);
	void Escape(const FInputActionValue& Value);
	void ToggleMenu(const FInputActionValue& Value);

	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnJumped_Implementation() override;

	//────────────────────────── 컴포넌트 ──────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHUDComponent* HUDComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractionComponent* InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComponent;

	UFUNCTION()
	void HandleDeath(AActor* DamageInstigator);

	// PlayerState.OnUpgradesChanged 구독 — 체력 재생 속도를 갱신한다.
	// (공격력/공격속도 배율은 매 발사 시점에 PlayerState 를 직접 조회하므로 여기서 처리하지 않는다.)
	UFUNCTION()
	void HandleUpgradesChanged();

	// PIE 디버그: 화면 좌상단에 일차/국면/체력/무기 데미지·연사속도/업그레이드 레벨을 표시.
	// #if ENABLE_DRAW_DEBUG 로 Shipping 빌드에서는 본문이 컴파일 제외됨.
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebugStatsOverlay = true;

	void DrawDebugStatsOverlay() const;

public:
	// 컴포넌트 접근자
	UFUNCTION(BlueprintPure, Category = "Components")
	UHUDComponent* GetHUDComponent() const { return HUDComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	//────────────────────────── 상태 변수 ──────────────────────────
public:
	bool bIsAiming = false;
	bool bIsFiring = false;

	float WalkSpeed = 450.f;

	UFUNCTION(BlueprintPure)
	bool GetbIsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetCurrentWeaponType() const;

	// AWeaponBase::Equip() 이 참조 — 플레이어는 몸통 소켓 대신 여기 붙는다.
	UFUNCTION(BlueprintPure, Category = "Components")
	USceneComponent* GetWeaponMount() const { return WeaponMount; }

	//────────────────────────── 카메라 / FOV (조준) ──────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// 힙파이어 시 무기가 향하는 목표. 카메라의 자식이라 항상 화면상 같은 자리(오른쪽)에
	// 무기가 보인다 (AWeaponBase::Equip, UpdateWeaponPose 참고).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* WeaponMount;

	// 조준(ADS) 시 무기의 스코프 소켓이 정렬되는 목표. 카메라 앞쪽에 위치해 총이 근접
	// 클리핑으로 사라지지 않게 하며, BP_KangPlayer 에서 조준선이 화면 중앙(크로스헤어)과
	// 맞도록 위치/회전을 직접 조정한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	USceneComponent* AimMount;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float DefaultFOV = 90.f;

	// 조준(ADS) 시 시야를 좁혀 에임 지점에 집중된 느낌을 준다.
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimInterpSpeed = 10.f;

	// 사격/재장전 몽타주 → UCombatComponent, 교체 몽타주 → UInventoryComponent 가 담당.
	// 이 클래스는 더 이상 무기 몽타주를 직접 재생하지 않는다.

	// 매 프레임 호출 — 무기(액터)의 실제 트랜스폼을 힙파이어(WeaponMount) 또는 조준
	// (AimMount) 목표로 보간한다. bIsAiming 과 무관하게 항상 호출되어 진입/해제 양쪽 모두
	// 스냅 없이 부드럽게 전환된다. Tick()에서만 호출된다.
	void UpdateWeaponPose(AWeaponBase* Weapon, float DeltaTime) const;
};
