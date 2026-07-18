// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Props/ShopItemData.h"
#include "ShopWidget.generated.h"

class UVerticalBox;
class AShop;

UCLASS()
class UE5_PORTFOLIO_1_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitShop(AShop* InShop);

	UFUNCTION(BlueprintCallable)
	void BuyItem(int32 ItemIndex);

	UFUNCTION(BlueprintCallable)
	TArray<FShopItemData> GetShopItems() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemList;

	UPROPERTY()
	AShop* Shop;
};
