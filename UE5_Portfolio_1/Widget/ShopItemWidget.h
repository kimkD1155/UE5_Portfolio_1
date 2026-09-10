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
	// DisplayPrice: 실제 표시 가격 (업그레이드는 레벨별로 계산된 값)
	// CurrentLevel/bMaxed: 업그레이드 항목의 현재 레벨과 최대치 여부 (무기/아군은 0, false)
	void InitItem(const FShopItemData& InItemData, int32 InItemIndex,
		int32 DisplayPrice, int32 CurrentLevel, bool bMaxed);

	UPROPERTY(BlueprintAssignable)
	FOnBuyClicked OnBuyClicked;

	// BP 에서 항목을 원하는 서식으로 표시하고 싶을 때 (레벨 배지 등). 선택적.
	UFUNCTION(BlueprintImplementableEvent, Category = "Shop")
	void OnItemInitialized(int32 DisplayPrice, int32 CurrentLevel, bool bMaxed);

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
