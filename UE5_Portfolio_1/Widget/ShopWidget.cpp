#include "ShopWidget.h"
#include "../Props/Shop.h"
#include "../Core/KangPlayerState.h"
#include "../Component/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/VerticalBox.h"
#include "../Core/KangPlayerController.h"

void UShopWidget::InitShop(AShop* InShop)
{
	Shop = InShop;
	if (!Shop || !ShopItemWidgetClass) return;

	RifleList->ClearChildren();
	PistolList->ClearChildren();
	AllyList->ClearChildren();

	for (int32 i = 0; i < Shop->ShopItems.Num(); i++)
	{
		UShopItemWidget* ItemWidget = CreateWidget<UShopItemWidget>(this, ShopItemWidgetClass);
		if (!ItemWidget) continue;

		ItemWidget->InitItem(Shop->ShopItems[i], i);
		ItemWidget->OnBuyClicked.AddDynamic(this, &UShopWidget::BuyItem);

		switch (Shop->ShopItems[i].ItemType)
		{
		case EShopItemType::Rifle:
			RifleList->AddChild(ItemWidget);
			break;
		case EShopItemType::Pistol:
			PistolList->AddChild(ItemWidget);
			break;
		case EShopItemType::Ally:
			AllyList->AddChild(ItemWidget);
			break;
		}
	}
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

	if ((Item.ItemType == EShopItemType::Rifle || Item.ItemType == EShopItemType::Pistol) && Item.WeaponClass)
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
	else if (Item.ItemType == EShopItemType::Ally && Item.AllyClass )
	{
		AKangPlayerController* KPC = Cast<AKangPlayerController>(GetWorld()->GetFirstPlayerController());
		if (KPC)
		{
			KPC->StartPlacementMode(Item.AllyClass);
			// »óÁ¡ ´Ý±â
			SetVisibility(ESlateVisibility::Hidden);
			Shop->SetShopOpen(false);
		}
	}
}