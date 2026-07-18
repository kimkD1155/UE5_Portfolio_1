// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoinWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UCoinWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateCoin(int32 CurrentCoin);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoinText;
};
