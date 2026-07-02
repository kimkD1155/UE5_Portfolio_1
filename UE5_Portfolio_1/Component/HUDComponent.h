// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Widget/InteractHintWidget.h"
#include "../Widget/CrosshairWidget.h"
#include "../Widget/AmmoWidget.h"
#include "../Widget/BarricadeWidget.h"
#include "HUDComponent.generated.h"

class ABarricade;

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

// 탄약 관련
	void UpdateAmmoUI(int32 CurrentAmmo, int32 ReserveAmmo, const FText& WeaponName);

// 바리케이드 관련
	void InitBarricadeUI(ABarricade* Barricade);
	UFUNCTION()
	void UpdateBarricadeUI(float CurrentHP, float MaxHP);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UInteractHintWidget> InteractHintWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UAmmoWidget> AmmoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UBarricadeWidget> BarricadeWidgetClass;

private:
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	UCrosshairWidget* CrosshairWidget = nullptr;
	UPROPERTY()
	UInteractHintWidget* InteractHintWidget = nullptr;
	UPROPERTY()
	UAmmoWidget* AmmoWidget = nullptr;
	UPROPERTY()
	UBarricadeWidget* BarricadeWidget = nullptr;
};
