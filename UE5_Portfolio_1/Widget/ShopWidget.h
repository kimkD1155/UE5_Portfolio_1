// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Props/ShopItemData.h"
#include "../Widget/ShopItemWidget.h"
#include "ShopWidget.generated.h"

class UVerticalBox;
class AShop;

UCLASS()
class UE5_PORTFOLIO_1_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopItemWidget> ShopItemWidgetClass;

	void InitShop(AShop* InShop);

	UFUNCTION(BlueprintCallable)
	void BuyItem(int32 ItemIndex);

	UFUNCTION(BlueprintCallable)
	TArray<FShopItemData> GetShopItems() const;

protected:

	UPROPERTY()
	AShop* Shop;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RifleList;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PistolList;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* AllyList;
};
