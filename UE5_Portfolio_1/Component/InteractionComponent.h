// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UHUDComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_PORTFOLIO_1_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// 무기 픽업 및 드랍
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
	UFUNCTION(BlueprintPure)
	AActor* GetCurrentInteractTarget() const { return CurrentInteractTarget; }

private:
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	UHUDComponent* HUDComp = nullptr;

	void UpdateInteractionTarget();   // Tick에서 호출

	UPROPERTY()
	AActor* CurrentInteractTarget = nullptr;   // 현재 조준 중인 대상
	UPROPERTY()
	AWeaponBase* EquippedWeapon = nullptr;
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
};
