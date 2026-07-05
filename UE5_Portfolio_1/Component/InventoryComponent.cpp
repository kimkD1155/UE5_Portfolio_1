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
	if (EquippedWeapon) DropWeapon();

	EquippedWeapon = Weapon;
	EquippedWeapon->Equip(OwnerCharacter);
	UE_LOG(LogTemp, Log, TEXT("Picked up weapon: %s"), *EquippedWeapon->GetName());
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

