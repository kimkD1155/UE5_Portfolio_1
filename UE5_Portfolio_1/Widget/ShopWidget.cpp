// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidget.h"
#include "../Props/Shop.h"
#include "../Core/KangPlayerState.h"
#include "../Component/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"


void UShopWidget::InitShop(AShop* InShop)
{
	Shop = InShop;
}

TArray<FShopItemData> UShopWidget::GetShopItems() const
{
	if (!Shop) return TArray<FShopItemData>();
	return Shop->ShopItems;
}

void UShopWidget::BuyItem(int32 ItemIndex)
{
	if (!Shop || !Shop->ShopItems.IsValidIndex(ItemIndex)) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	AKangPlayerState* PS = PC->GetPlayerState<AKangPlayerState>();
	if (!PS) return;

	FShopItemData& Item = Shop->ShopItems[ItemIndex];

	if (!PS->SpendCoin(Item.Price))
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough coin: %d / %d"), PS->GetCoin(), Item.Price);
		return;
	}

	if (Item.ItemType == EShopItemType::Weapon && Item.WeaponClass)
	{
		ACharacter* Player = Cast<ACharacter>(PC->GetPawn());
		if (!Player) return;

		AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(
			Item.WeaponClass,
			Player->GetActorLocation(),
			FRotator::ZeroRotator
		);

		if (Weapon)
		{
			UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
			if (Inventory) Inventory->PickupWeapon(Weapon);
			UE_LOG(LogTemp, Warning, TEXT("Bought: %s"), *Item.ItemName.ToString());
		}
	}
}