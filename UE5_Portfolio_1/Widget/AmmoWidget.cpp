// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoWidget.h"

void UAmmoWidget::UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo)
{
    if (AmmoText)
    {
        FString Text = FString::Printf(TEXT("%d / %d"), CurrentAmmo, ReserveAmmo);
        AmmoText->SetText(FText::FromString(Text));
    }
}

void UAmmoWidget::UpdateWeaponName(const FText& WeaponName)
{
    if (WeaponNameText)
        WeaponNameText->SetText(WeaponName);
}

void UAmmoWidget::ShowAmmoUI()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UAmmoWidget::HideAmmoUI()
{
    SetVisibility(ESlateVisibility::Hidden);
}