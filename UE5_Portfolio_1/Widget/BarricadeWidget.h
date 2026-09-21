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

UCLASS()
class UE5_PORTFOLIO_1_API UBarricadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHP(float CurrentHP, float MaxHP);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;
};
