// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Props/ShopItemData.h"
#include "ShopItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuyClicked, int32, ItemIndex);
/**
 * 
 */
class UTextBlock;
class UButton;

UCLASS()
class UE5_PORTFOLIO_1_API UShopItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitItem(const FShopItemData& InItemData, int32 InItemIndex);

	UPROPERTY(BlueprintAssignable)
	FOnBuyClicked OnBuyClicked;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBuyButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PriceText;

	UPROPERTY(meta = (BindWidget))
	UButton* BuyButton;

	FShopItemData ItemData;
	int32 ItemIndex;
};
