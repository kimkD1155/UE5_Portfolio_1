// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoWidget.h"

void UAmmoWidget::UpdateAmmo(int32 CurrentAmmo, int32 ReserveAmmo)
{
    if (AmmoText)
    {
        // 예비 탄약은 무한이라(ARangedWeapon::ReloadFinished 참고) 항상 ∞ 로 표시한다.
        FString Text = FString::Printf(TEXT("%d / %s"), CurrentAmmo, TEXT("∞"));
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