// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Weapon/WeaponBase.h"
#include "KangPlayerCharacter.generated.h"


class UInputAction;
struct FInputActionValue;

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


protected:
	// 키 입력 액션
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

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Drop(const FInputActionValue& Value);
	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);
	void StartAim(const FInputActionValue& Value);
	void StopAim(const FInputActionValue& Value);

protected:
	// 라인트레이스 감지 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractTraceDistance = 600.f;


public:
	// 무기 픽업, 드랍
	void PickupWeapon(AWeaponBase* Weapon);
	void DropWeapon();

	UFUNCTION(BlueprintPure)
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

private:
	void UpdateInteractionTarget();   // Tick에서 호출

	UPROPERTY()
	AActor* CurrentInteractTarget = nullptr;   // 현재 조준 중인 대상

	UPROPERTY()
	AWeaponBase* EquippedWeapon = nullptr;
};

