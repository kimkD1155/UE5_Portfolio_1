// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractHintWidget.h"
#include "Components/TextBlock.h"

void UInteractHintWidget::SetHintText(const FText& Text)
{
    HintText = Text;
    if (HintTextBlock)
    {
        HintTextBlock->SetText(HintText);
	}
}

void UInteractHintWidget::ShowHint()
{
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UInteractHintWidget::HideHint()
{
    SetVisibility(ESlateVisibility::Hidden);
}