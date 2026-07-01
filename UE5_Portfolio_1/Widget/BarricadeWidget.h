// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BarricadeWidget.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;
class ABarricade;

UCLASS()
class UE5_PORTFOLIO_1_API UBarricadeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitWidget(ABarricade* InBarricade);
	void UpdateHP(float CurrentHP, float MaxHP);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;

	UPROPERTY()
	ABarricade* Barricade;

};
