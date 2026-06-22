// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "AmmoWidget.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo);
    void UpdateWeaponName(const FText& WeaponName);
    void ShowAmmoUI();
    void HideAmmoUI();
	
protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AmmoText;


    UPROPERTY(meta = (BindWidget))
    UTextBlock* WeaponNameText;
};
