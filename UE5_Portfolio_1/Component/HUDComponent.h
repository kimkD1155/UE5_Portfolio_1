// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Widget/InteractHintWidget.h"
#include "../Widget/CrosshairWidget.h"
#include "../Widget/AmmoWidget.h"
#include "../Widget/BarricadeWidget.h"
#include "../Widget/CoinWidget.h"
#include "../Widget/PhaseWidget.h"
#include "../Widget/ResultWidget.h"
#include "../Core/GamePhase.h"
#include "HUDComponent.generated.h"

class ABarricade;
class AWeaponBase;
class ARangedWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_PORTFOLIO_1_API UHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHUDComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
public:
// 상호작용 힌트 관련
	void ShowInteractHint(const FText& Text);
	void HideInteractHint();

// 탄약 관련 — InventoryComponent.OnWeaponEquipped / RangedWeapon.OnAmmoChanged 에 바인딩
	void UpdateAmmoUI(int32 CurrentAmmo, int32 ReserveAmmo, const FText& WeaponName);

	UFUNCTION()
	void HandleWeaponEquipped(AWeaponBase* NewWeapon);

	UFUNCTION()
	void HandleAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo);

// 바리케이드 관련
	void InitBarricadeUI(ABarricade* Barricade);
	UFUNCTION()
	void UpdateBarricadeUI(float CurrentHP, float MaxHP);

// 코인 관련
	UFUNCTION()
	void UpdateCoinUI(int32 CurrentCoin);

// 국면(낮/밤) 관련 — AKangGameState 델리게이트에 바인딩
	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	UFUNCTION()
	void HandlePhaseTimeChanged(float Remaining);

	// UEnemyManager::OnEnemyCountChanged (비-다이나믹) 바인딩
	void HandleEnemyCountChanged(int32 NewCount);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UInteractHintWidget> InteractHintWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UAmmoWidget> AmmoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UBarricadeWidget> BarricadeWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UCoinWidget> CoinWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UPhaseWidget> PhaseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UResultWidget> ResultWidgetClass;

private:
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	UCrosshairWidget* CrosshairWidget = nullptr;
	UPROPERTY()
	UInteractHintWidget* InteractHintWidget = nullptr;
	UPROPERTY()
	UAmmoWidget* AmmoWidget = nullptr;
	// 현재 탄약 델리게이트가 바인딩된 무기 (무기 교체 시 갈아탐)
	UPROPERTY()
	ARangedWeapon* BoundAmmoWeapon = nullptr;
	UPROPERTY()
	UBarricadeWidget* BarricadeWidget = nullptr;
	UPROPERTY()
	UCoinWidget* CoinWidget = nullptr;
	UPROPERTY()
	UPhaseWidget* PhaseWidget = nullptr;
	UPROPERTY()
	UResultWidget* ResultWidget = nullptr;
};
