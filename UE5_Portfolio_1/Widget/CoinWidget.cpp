// Fill out your copyright notice in the Description page of Project Settings.


#include "CoinWidget.h"
#include "Components/TextBlock.h"

void UCoinWidget::UpdateCoin(int32 CurrentCoin)
{
	if (!CoinText) return;
	CoinText->SetText(FText::FromString(FString::Printf(TEXT("$ %d"), CurrentCoin)));
}