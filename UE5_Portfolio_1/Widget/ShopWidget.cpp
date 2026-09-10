#include "ShopWidget.h"
#include "../Props/Shop.h"
#include "../Core/KangPlayerState.h"
#include "../Component/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/VerticalBox.h"
#include "../Core/KangPlayerController.h"
#include "../Manager/AllyManager.h"
#include "../Ally/AllySpawnPoint.h"

void UShopWidget::InitShop(AShop* InShop)
{
	Shop = InShop;
	if (!Shop || !ShopItemWidgetClass) return;

	RifleList->ClearChildren();
	PistolList->ClearChildren();
	AllyList->ClearChildren();
	if (UpgradeList) UpgradeList->ClearChildren();

	AKangPlayerState* PS = nullptr;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PS = PC->GetPlayerState<AKangPlayerState>();
	}

	for (int32 i = 0; i < Shop->ShopItems.Num(); i++)
	{
		const FShopItemData& Item = Shop->ShopItems[i];

		UShopItemWidget* ItemWidget = CreateWidget<UShopItemWidget>(this, ShopItemWidgetClass);
		if (!ItemWidget) continue;

		int32 DisplayPrice = Item.Price;
		int32 CurrentLevel = 0;
		bool bMaxed = false;
		if (Item.ItemType == EShopItemType::Upgrade && PS)
		{
			CurrentLevel = PS->GetUpgradeLevel(Item.UpgradeType);
			bMaxed = PS->IsUpgradeMaxed(Item.UpgradeType);
			DisplayPrice = PS->GetUpgradeCost(Item.UpgradeType);
		}

		ItemWidget->InitItem(Item, i, DisplayPrice, CurrentLevel, bMaxed);
		ItemWidget->OnBuyClicked.AddDynamic(this, &UShopWidget::BuyItem);

		switch (Item.ItemType)
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
		case EShopItemType::Upgrade:
			if (UpgradeList) UpgradeList->AddChild(ItemWidget);
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

	// 업그레이드는 비용이 레벨에 따라 달라지므로 PlayerState 가 비용 계산과 코인 차감을 모두 처리한다.
	if (Item.ItemType == EShopItemType::Upgrade)
	{
		PS->TryPurchaseUpgrade(Item.UpgradeType);
		InitShop(Shop); // 레벨/다음 비용 표시 갱신
		return;
	}

	if (!PS->SpendCoin(Item.Price))
	{

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
			
		}
	}
	else if (Item.ItemType == EShopItemType::Ally && Item.AllyClass )
	{
		UAllyManager* AllyManager = GetWorld()->GetSubsystem<UAllyManager>();
		if (!AllyManager) return;

		AAllySpawnPoint* SpawnPoint = AllyManager->GetAvailableSpawnPoint();
		if (!SpawnPoint)
		{
			UE_LOG(LogTemp, Warning, TEXT("No available ally spawn point"));
			PS->AddCoin(Item.Price);
			return;
		}

		AAllyBase* NewAlly = GetWorld()->SpawnActor<AAllyBase>(
			Item.AllyClass,
			SpawnPoint->GetActorLocation(),
			SpawnPoint->GetActorRotation()
		);

		if (NewAlly)
		{
			SpawnPoint->bIsOccupied = true;
			UE_LOG(LogTemp, Warning, TEXT("Bought Ally: %s"), *Item.ItemName.ToString());
		}
	}
}
