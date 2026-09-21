// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Weapon/WeaponBase.h"
#include "../Animation/MontageHelper.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	// 기본 무기 지급
	if (DefaultWeaponClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;
		AWeaponBase* DefaultWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, Params);
		PickupWeapon(DefaultWeapon);
	}
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon || !OwnerCharacter) return;

	Weapon->SetOwner(OwnerCharacter);

	// 총은 한 번에 하나만 소유한다 — 슬롯에 상관없이 기존에 들고 있던 무기를 전부 정리한다.
	// EquippedWeapon 도 같이 비워야 한다: 그중 하나가 지금 장착 중인 무기라면 Destroy() 직후에도
	// EquippedWeapon 포인터는 즉시 null이 되지 않아, 아래 EquipSlot()의 홀스터 분기가 파괴된
	// 액터를 참조하게 되는 문제가 있다.
	for (const TPair<EWeaponSlot, AWeaponBase*>& Pair : WeaponSlots)
	{
		if (AWeaponBase* Existing = Pair.Value)
		{
			Existing->Unequip();
			Existing->Destroy();
		}
	}
	WeaponSlots.Empty();
	EquippedWeapon = nullptr;

	// 무기가 자기 슬롯을 알고 있다 (인벤토리는 무기 타입 분기 안 함, OCP)
	const EWeaponSlot Slot = Weapon->GetPreferredSlot();
	WeaponSlots.Add(Slot, Weapon);
	EquipSlot(Slot);
}

void UInventoryComponent::DropWeapon()
{
	if (!EquippedWeapon || !OwnerCharacter) return;

	EquippedWeapon->Unequip();
	const FVector DropLocation = OwnerCharacter->GetActorLocation()
		+ OwnerCharacter->GetActorForwardVector() * 100.f
		+ FVector(0.f, 0.f, -50.f);
	EquippedWeapon->SetActorLocation(DropLocation);

	WeaponSlots.Remove(CurrentSlot);
	EquippedWeapon = nullptr;
}

void UInventoryComponent::EquipSlot(EWeaponSlot Slot)
{
	AWeaponBase** FoundPtr = WeaponSlots.Find(Slot);
	if (!FoundPtr || !*FoundPtr || !OwnerCharacter) return;

	// 들고 있던 무기는 홀스터 소켓으로
	if (EquippedWeapon)
	{
		if (FName* HolsterSocket = HolsterSocketMap.Find(CurrentSlot))
		{
			EquippedWeapon->AttachToComponent(OwnerCharacter->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, *HolsterSocket);
		}
	}

	EquippedWeapon = *FoundPtr;
	CurrentSlot = Slot;

	// 교체 몽타주: 무기의 AnimSet 에서 가져온다. 없으면 즉시 완료.
	bIsEquipping = true;
	UAnimMontage* EquipMontage = EquippedWeapon->GetCharacterEquipMontage();
	const float Len = MontageHelper::PlayWithEndCallback(
		OwnerCharacter->GetMesh(), EquipMontage, this, &UInventoryComponent::OnEquipMontageEnded);
	if (Len <= 0.f)
	{
		bIsEquipping = false;
	}

	EquippedWeapon->Equip(OwnerCharacter);

	OnWeaponEquipped.Broadcast(EquippedWeapon);
}

void UInventoryComponent::OnEquipMontageEnded(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
	bIsEquipping = false;
}
