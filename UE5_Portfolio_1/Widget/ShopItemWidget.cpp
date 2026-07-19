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

void UShopItemWidget::InitItem(const FShopItemData& InItemData, int32 InItemIndex)
{
	ItemData = InItemData;
	ItemIndex = InItemIndex;

	if (ItemNameText)
		ItemNameText->SetText(ItemData.ItemName);

	if (PriceText)
		PriceText->SetText(FText::FromString(FString::Printf(TEXT("$ %d"), ItemData.Price)));
}

void UShopItemWidget::OnBuyButtonClicked()
{
	OnBuyClicked.Broadcast(ItemIndex);
}