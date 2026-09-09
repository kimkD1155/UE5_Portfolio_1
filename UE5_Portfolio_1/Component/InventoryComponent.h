// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Weapon/WeaponSlot.h"
#include "InventoryComponent.generated.h"

class AWeaponBase;
class ACharacter;
class UAnimMontage;

// 장착 무기가 바뀔 때 브로드캐스트 (HUD 가 새 무기의 탄약 델리게이트로 갈아타도록)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, AWeaponBase*, Weapon);

/**
 * 무기 슬롯 관리 + 장착/드롭 + 교체 몽타주.
 * 소유자는 구체 캐릭터 클래스가 아니라 ACharacter 로만 다루므로
 * 어떤 캐릭터(플레이어/아군)에도 재사용 가능하다. (DIP)
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE5_PORTFOLIO_1_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PickupWeapon(AWeaponBase* Weapon);
	void DropWeapon();
	void EquipSlot(EWeaponSlot Slot);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetIsEquipping() const { return bIsEquipping; }

	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	EWeaponSlot GetCurrentSlot() const { return CurrentSlot; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TMap<EWeaponSlot, FName> HolsterSocketMap;

protected:
	void OnEquipMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY()
	ACharacter* OwnerCharacter;

	bool bIsEquipping = false;

	UPROPERTY()
	AWeaponBase* EquippedWeapon;

	UPROPERTY()
	TMap<EWeaponSlot, AWeaponBase*> WeaponSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	EWeaponSlot CurrentSlot = EWeaponSlot::Primary;
};
