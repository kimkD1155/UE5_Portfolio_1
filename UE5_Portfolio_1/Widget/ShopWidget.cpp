#include "ShopWidget.h"
#include "../Core/KangPlayerState.h"
#include "../Component/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/VerticalBox.h"
#include "../Core/KangPlayerController.h"
#include "../Manager/AllyManager.h"
#include "../Character/Ally/AllyBase.h"
#include "../Character/Ally/AllySpawnPoint.h"
#include "../Core/KangPlayerGameModeBase.h"

void UShopWidget::RefreshCatalog()
{
	if (!ShopItemWidgetClass) return;

	RifleList->ClearChildren();
	PistolList->ClearChildren();
	AllyList->ClearChildren();
	if (UpgradeList) UpgradeList->ClearChildren();

	AKangPlayerState* PS = nullptr;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PS = PC->GetPlayerState<AKangPlayerState>();
	}

	for (int32 i = 0; i < ShopItems.Num(); i++)
	{
		const FShopItemData& Item = ShopItems[i];

		UShopItemWidget* ItemWidget = CreateWidget<UShopItemWidget>(this, ShopItemWidgetClass);
		if (!ItemWidget) continue;

		int32 DisplayPrice = Item.Price;
		int32 CurrentLevel = 0;
		bool bMaxed = false;
		bool bAlreadyOwned = false;
		if (Item.ItemType == EShopItemType::Upgrade && PS)
		{
			CurrentLevel = PS->GetUpgradeLevel(Item.UpgradeType);
			bMaxed = PS->IsUpgradeMaxed(Item.UpgradeType);
			DisplayPrice = PS->GetUpgradeCost(Item.UpgradeType);
		}
		else if ((Item.ItemType == EShopItemType::Rifle || Item.ItemType == EShopItemType::Pistol)
			&& Item.WeaponClass && PS && PS->IsWeaponUnlocked(Item.WeaponClass))
		{
			// 한 번 구매해 해금한 무기는 이후 계속 무료로 재구매(재장착)할 수 있다.
			DisplayPrice = 0;
			bAlreadyOwned = true;
		}

		ItemWidget->InitItem(Item, i, DisplayPrice, CurrentLevel, bMaxed, bAlreadyOwned);
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

	// 동료 무기 교체 목록: 현재 활성 동료 x 무기 옵션의 조합
	if (AllyWeaponList)
	{
		AllyWeaponList->ClearChildren();

		UAllyManager* AllyMgr = GetWorld()->GetSubsystem<UAllyManager>();
		const TArray<AAllyBase*> Allies = AllyMgr ? AllyMgr->GetActiveAllies() : TArray<AAllyBase*>();

		for (int32 AllyIdx = 0; AllyIdx < Allies.Num(); ++AllyIdx)
		{
			for (int32 OptIdx = 0; OptIdx < AllyWeaponOptions.Num(); ++OptIdx)
			{
				const FAllyWeaponOption& Option = AllyWeaponOptions[OptIdx];

				UShopItemWidget* ItemWidget = CreateWidget<UShopItemWidget>(this, ShopItemWidgetClass);
				if (!ItemWidget) continue;

				// 동료 번호를 이름 앞에 붙여 어떤 동료용 항목인지 구분한다.
				FShopItemData Display;
				Display.ItemName = FText::FromString(
					FString::Printf(TEXT("동료 %d - %s"), AllyIdx + 1, *Option.ItemName.ToString()));
				Display.Price = Option.Price;

				const int32 CombinedIndex = AllyIdx * AllyWeaponOptions.Num() + OptIdx;
				ItemWidget->InitItem(Display, CombinedIndex, Option.Price, 0, false);
				ItemWidget->OnBuyClicked.AddDynamic(this, &UShopWidget::BuyAllyWeapon);

				AllyWeaponList->AddChild(ItemWidget);
			}
		}
	}
}

void UShopWidget::BuyItem(int32 ItemIndex)
{
	if (!ShopItems.IsValidIndex(ItemIndex)) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	AKangPlayerState* PS = PC->GetPlayerState<AKangPlayerState>();
	if (!PS) return;

	FShopItemData& Item = ShopItems[ItemIndex];

	// 업그레이드는 비용이 레벨에 따라 달라지므로 PlayerState 가 비용 계산과 코인 차감을 모두 처리한다.
	if (Item.ItemType == EShopItemType::Upgrade)
	{
		PS->TryPurchaseUpgrade(Item.UpgradeType);
		RefreshCatalog(); // 레벨/다음 비용 표시 갱신
		return;
	}

	const bool bIsWeapon = (Item.ItemType == EShopItemType::Rifle || Item.ItemType == EShopItemType::Pistol) && Item.WeaponClass;
	// 이미 해금한 무기는 재구매(재장착) 시 코인을 받지 않는다.
	const bool bFreeRepurchase = bIsWeapon && PS->IsWeaponUnlocked(Item.WeaponClass);

	if (!bFreeRepurchase && !PS->SpendCoin(Item.Price))
	{
		return;
	}

	if (bIsWeapon)
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
			// 총은 한 번에 하나만 소유 — PickupWeapon 이 기존에 들고 있던 무기를 전부 정리하고 이걸로 대체한다.
			UInventoryComponent* Inventory = Player->FindComponentByClass<UInventoryComponent>();
			if (Inventory) Inventory->PickupWeapon(Weapon);
			PS->UnlockWeapon(Item.WeaponClass);
			RefreshCatalog(); // 방금 해금됐으니 가격 표시("무료")를 즉시 반영
		}
	}
	else if (Item.ItemType == EShopItemType::Ally && Item.AllyClass)
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
			RefreshCatalog(); // 새 동료가 무기 교체 목록에도 나타나도록
		}
	}
}

void UShopWidget::BuyAllyWeapon(int32 CombinedIndex)
{
	if (AllyWeaponOptions.Num() == 0) return;

	const int32 AllyIdx = CombinedIndex / AllyWeaponOptions.Num();
	const int32 OptIdx = CombinedIndex % AllyWeaponOptions.Num();
	if (!AllyWeaponOptions.IsValidIndex(OptIdx)) return;

	UAllyManager* AllyMgr = GetWorld()->GetSubsystem<UAllyManager>();
	const TArray<AAllyBase*> Allies = AllyMgr ? AllyMgr->GetActiveAllies() : TArray<AAllyBase*>();
	if (!Allies.IsValidIndex(AllyIdx)) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	AKangPlayerState* PS = PC ? PC->GetPlayerState<AKangPlayerState>() : nullptr;
	if (!PS) return;

	const FAllyWeaponOption& Option = AllyWeaponOptions[OptIdx];
	if (!PS->SpendCoin(Option.Price)) return;

	Allies[AllyIdx]->SwapWeapon(Option.WeaponClass);
	RefreshCatalog();
}

void UShopWidget::StartNight()
{
	if (AKangPlayerGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>() : nullptr)
	{
		GM->RequestStartNight();
	}
}
