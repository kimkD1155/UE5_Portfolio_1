// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopItemWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UShopItemWidget::OnBuyButtonClicked);
	}
}

void UShopItemWidget::InitItem(const FShopItemData& InItemData, int32 InItemIndex,
	int32 DisplayPrice, int32 CurrentLevel, bool bMaxed)
{
	ItemData = InItemData;
	ItemIndex = InItemIndex;

	if (ItemNameText)
		ItemNameText->SetText(ItemData.ItemName);

	if (PriceText)
	{
		const FString PriceStr = bMaxed
			? FString(TEXT("MAX"))
			: FString::Printf(TEXT("$ %d"), DisplayPrice);
		PriceText->SetText(FText::FromString(PriceStr));
	}

	if (BuyButton)
		BuyButton->SetIsEnabled(!bMaxed);

	OnItemInitialized(DisplayPrice, CurrentLevel, bMaxed);
}

void UShopItemWidget::OnBuyButtonClicked()
{
	OnBuyClicked.Broadcast(ItemIndex);
}