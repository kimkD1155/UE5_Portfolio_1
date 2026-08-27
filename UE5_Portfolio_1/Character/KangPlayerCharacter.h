// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KangPlayerCharacter.generated.h"

//ㅡㅡㅡㅡㅡㅡㅡ전방 선언ㅡㅡㅡㅡㅡㅡ
// 컴포넌트
class UHUDComponent;
class UInteractionComponent;
class UInventoryComponent;

class AWeaponBase;
class ARangedWeapon;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
struct FInputActionValue;

DECLARE_DELEGATE_TwoParams(FOnMontageEnded, UAnimMontage*, bool /*bInterrupted*/);

UENUM(BlueprintType)
enum class ETurnDirection : uint8
{
	None    UMETA(DisplayName = "None"),
	Left    UMETA(DisplayName = "Left"),
	Right   UMETA(DisplayName = "Right")
};

UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AKangPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 키 입력 관련
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
//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 컴포넌트
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHUDComponent* HUDComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInteractionComponent* InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;

public:
	// 컴포넌트 접근자
	UFUNCTION(BlueprintPure, Category = "Components")
	UHUDComponent* GetHUDComponent() const { return HUDComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UFUNCTION(BlueprintPure, Category = "Components")
	UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 변수
public:
	bool bIsAiming = false;
	bool bIsFiring = false;
	bool bIsEquipping = false;
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
	UPROPERTY()
	ARangedWeapon* ReloadingWeapon;
	
//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 카메라, FOV 처리 ( AIm )
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


//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 몽타주 및 애니메이션 관련
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> PistolReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> PistolFireMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> PistolEquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RifleReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RifleFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RifleEquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ShotgunReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ShotgunFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ShotgunEquipMontage;

	void PlayReloadMontage(UAnimMontage* MontageToPlay);
	void PlayEquipMontage(UAnimMontage* MontageToPlay);



public:
	UFUNCTION()
	void OnReloadNotify();
	UFUNCTION()
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);


//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//IK 관련
public:
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FTransform LeftHandIKTarget;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	bool bShouldUseLeftHandIK = false;

	void UpdateLeftHandIK();


};