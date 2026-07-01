// Fill out your copyright notice in the Description page of Project Settings.


#include "BarricadeWidget.h"
#include "../Props/Barricade.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBarricadeWidget::InitWidget(ABarricade* InBarricade)
{
	Barricade = InBarricade;
	UpdateHP(Barricade->GetCurrentHealth(), Barricade->GetMaxHealth());
}

void UBarricadeWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPBar)
		HPBar->SetPercent(CurrentHP / MaxHP);

	if (HPText)
	{
		FString Text = FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP);
		HPText->SetText(FText::FromString(Text));
	}
}