// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Weapon/WeaponBase.h"          // EWeaponType
#include "../Interface/WeaponHolder.h"
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

UENUM(BlueprintType)
enum class ETurnDirection : uint8
{
	None    UMETA(DisplayName = "None"),
	Left    UMETA(DisplayName = "Left"),
	Right   UMETA(DisplayName = "Right")
};

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
	bool Turn = false;

	UPROPERTY(BlueprintReadOnly, Category = "Turn")
	ETurnDirection TurnDirection = ETurnDirection::None;

	float WalkSpeed = 450.f;
	float AimYaw = 0.f;

	UPROPERTY(EditAnywhere, Category = "Aim")
	float MovingSpeedThreshold = 10.f; // 이 이상이면 "이동 중"으로 판정

	bool bIsMoving = false;

	void UpdateMovementState();
	UFUNCTION(BlueprintPure)
	bool GetbIsAiming() const { return bIsAiming; }
	UFUNCTION(BlueprintPure)
	bool GetIsMoving() const { return bIsMoving; }
	UFUNCTION(BlueprintPure)
	float GetAimYaw() const { return AimYaw; }
	UFUNCTION(BlueprintPure)
	bool CanTurn() const { return Turn; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetCurrentWeaponType() const;
	UFUNCTION(BlueprintPure)
	ETurnDirection GetTurnDirection() const { return TurnDirection; }
	UFUNCTION(BlueprintPure)
	ETurnDirection SetTurnDirection(ETurnDirection TD) { return TurnDirection = TD; }

	//────────────────────────── 카메라 / FOV (조준) ──────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	float AimInterpSpeed = 10.f;

	// 사격/재장전 몽타주 → UCombatComponent, 교체 몽타주 → UInventoryComponent 가 담당.
	// 이 클래스는 더 이상 무기 몽타주를 직접 재생하지 않는다.

	//────────────────────────── 왼손 IK ──────────────────────────
public:
	UPROPERTY(BlueprintReadWrite, Category = "IK")
	FTransform LeftHandIKTarget;

	UPROPERTY(BlueprintReadWrite, Category = "IK")
	bool bShouldUseLeftHandIK = false;

	void UpdateLeftHandIK();
};
