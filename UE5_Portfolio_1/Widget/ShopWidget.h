// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Props/ShopItemData.h"
#include "../Widget/ShopItemWidget.h"
#include "ShopWidget.generated.h"

class UVerticalBox;
class AAllyBase;

// 동료에게 사줄 수 있는 무기 한 종류. FShopItemData 와 달리 구매 대상이
// "플레이어"가 아니라 "동료 한 명"이라 별도 구조체로 둔다.
USTRUCT(BlueprintType)
struct FAllyWeaponOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AWeaponBase> WeaponClass;
};

/**
 * B 키로 여는 커맨드 메뉴. 예전에는 AShop 액터에 상호작용해야 열렸지만,
 * 이제 액터 없이 이 위젯 하나로 구매/수색/동료 무기 교체를 전부 처리한다.
 * 카탈로그(ShopItems, AllyWeaponOptions)는 위젯 클래스 디폴트에서 편집한다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopItemWidget> ShopItemWidgetClass;

	// 무기/아군/업그레이드 카탈로그. 예전엔 AShop 인스턴스가 들고 있었다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FShopItemData> ShopItems;

	// 동료에게 사줄 수 있는 무기 목록 (동료 종류와 무관하게 공통 카탈로그).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FAllyWeaponOption> AllyWeaponOptions;

	// 메뉴를 열 때마다 호출 — 목록/가격/레벨을 최신 상태로 다시 그린다.
	UFUNCTION(BlueprintCallable)
	void RefreshCatalog();

	UFUNCTION(BlueprintCallable)
	void BuyItem(int32 ItemIndex);

	// CombinedIndex = AllyIndex * AllyWeaponOptions.Num() + OptionIndex
	UFUNCTION(BlueprintCallable)
	void BuyAllyWeapon(int32 CombinedIndex);

	UFUNCTION(BlueprintCallable)
	TArray<FShopItemData> GetShopItems() const { return ShopItems; }

	//────────────────────────── 수색 (구 AScavengePoint) ──────────────────────────
	// 낮에만, 기본적으로 하루 1회 코인을 지급한다. 버튼 연결은 위젯 BP 쪽에서 한다.
	UFUNCTION(BlueprintCallable, Category = "Scavenge")
	bool Scavenge();

	UFUNCTION(BlueprintPure, Category = "Scavenge")
	bool CanScavengeNow() const;

	UFUNCTION(BlueprintPure, Category = "Scavenge")
	FText GetScavengeHintText() const;

	// 낮 국면이 새로 시작될 때 HUDComponent 가 호출 — 하루 1회 제한을 초기화한다.
	void ResetDailyScavenge() { bScavengedToday = false; }

protected:
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RifleList;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PistolList;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* AllyList;

	// 업그레이드 항목용. 기존 상점 BP 를 깨지 않도록 선택적 바인딩.
	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* UpgradeList;

	// 동료 무기 교체 목록. 이번에 추가된 기능이라 선택적 바인딩.
	UPROPERTY(meta = (BindWidgetOptional))
	UVerticalBox* AllyWeaponList;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenge", meta = (ClampMin = "0"))
	int32 ScavengeCoinReward = 40;

	// true = 낮마다 1회. false = 낮 동안 무제한.
	UPROPERTY(EditDefaultsOnly, Category = "Scavenge")
	bool bScavengeOncePerDay = true;

	bool bScavengedToday = false;
};
