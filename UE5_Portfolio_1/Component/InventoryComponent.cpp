// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "GameFramework/Character.h"
#include "../Weapon/WeaponBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	// ...
	// 기본 보조무기(권총) 지급
	if (DefaultWeaponClass && OwnerCharacter)
	{
		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;
		AWeaponBase* DefaultWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);
		PickupWeapon(DefaultWeapon);
	}
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



void UInventoryComponent::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return;

	const EWeaponSlot Slot = (Weapon->GetWeaponType() == EWeaponType::Pistol) ? EWeaponSlot::Secondary :
		(Weapon->GetWeaponType() == EWeaponType::Rifle) ? EWeaponSlot::Primary : EWeaponSlot::Throwable;


	// 해당 슬롯에 기존 무기 있으면 제거(드롭 또는 파괴)
	if (AWeaponBase** ExistingPtr = WeaponSlots.Find(Slot))
	{
		if (AWeaponBase* Existing = *ExistingPtr)
		{
			Existing->Unequip();
			Existing->Destroy(); // 필요하면 드롭 로직으로 교체
		}
	}

	WeaponSlots.Add(Slot, Weapon);

	// 픽업한 무기가 현재 슬롯이면 바로 장착
	if (Slot == CurrentSlot)
	{
		EquipSlot(Slot);
	}

	UE_LOG(LogTemp, Log, TEXT("Picked up weapon: %s (Slot: %d)"), *Weapon->GetName(), (int32)Slot);
}

void UInventoryComponent::DropWeapon()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->Unequip();
	FVector DropLocation = OwnerCharacter->GetActorLocation()
		+ OwnerCharacter->GetActorForwardVector() * 100.f
		+ FVector(0.f, 0.f, -50.f);
	EquippedWeapon->SetActorLocation(DropLocation);
	EquippedWeapon = nullptr;
}

void UInventoryComponent::EquipSlot(EWeaponSlot Slot)
{
	AWeaponBase** FoundPtr = WeaponSlots.Find(Slot);
	if (!FoundPtr || !*FoundPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No weapon found in slot: %d"), (int32)Slot);
		return;
	}

	if (EquippedWeapon)
	{
		UE_LOG(LogTemp, Log, TEXT("Unequipping weapon: %s (Slot: %d)"), *EquippedWeapon->GetName(), (int32)CurrentSlot);
		EquippedWeapon->Unequip();
	}
	UE_LOG(LogTemp, Log, TEXT("Equipping weapon: %s (Slot: %d)"), *(*FoundPtr)->GetName(), (int32)Slot);
	EquippedWeapon = *FoundPtr;
	EquippedWeapon->Equip(OwnerCharacter);
	CurrentSlot = Slot;
}