// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"



class AWeaponBase;
class AKangPlayerCharacter;

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Primary,
	Secondary,
	Throwable
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_PORTFOLIO_1_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:

	void PickupWeapon(AWeaponBase* Weapon);
	void DropWeapon();
	void EquipSlot(EWeaponSlot Slot);
	bool GetIsEquipping() const { return bIsEquipping; }

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HolsterPrimaryName = "Primary_Holster";

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName HolsterSecondaryName = "Secondary_Holster";

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TMap<EWeaponSlot, FName> HolsterSocketMap;

	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	EWeaponSlot GetCurrentSlot() const { return CurrentSlot; }

protected:

	UPROPERTY()
	AKangPlayerCharacter* OwnerCharacter;

	bool bIsEquipping = false;

	UPROPERTY()
	AWeaponBase* EquippedWeapon;

	UPROPERTY()
	TMap<EWeaponSlot, AWeaponBase*> WeaponSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	EWeaponSlot CurrentSlot = EWeaponSlot::Primary;

};
