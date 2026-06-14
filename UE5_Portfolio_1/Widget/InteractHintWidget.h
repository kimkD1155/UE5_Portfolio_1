// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractHintWidget.generated.h"

/**
 * 
 */
class UTextBlock;

UCLASS()
class UE5_PORTFOLIO_1_API UInteractHintWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    void SetHintText(const FText& Text);
    void ShowHint();
    void HideHint();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "HUD")
    FText HintText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HintTextBlock;
};
