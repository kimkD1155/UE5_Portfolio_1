// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "../Component/InventoryComponent.h"
#include "../Character/KangPlayerCharacter.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 무기 메시를 루트로
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    WeaponMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 라인트레이스 채널만 허용


}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// ── 장착 ──────────────────────────────────────────────────────────────────────
void AWeaponBase::Equip(ACharacter* NewOwner)
{
	UE_LOG(LogTemp, Log, TEXT("Equipping weapon"));
    OwnerCharacter = NewOwner;
    SetOwner(NewOwner);

    //// 어떤 메시에 붙으려는지 확인
    //USkeletalMeshComponent* CharMesh = NewOwner->GetMesh();
    //UE_LOG(LogTemp, Warning, TEXT("Mesh: %s"), *CharMesh->GetName());
    //UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh Asset: %s"),
    //    CharMesh->GetSkeletalMeshAsset()
    //    ? *CharMesh->GetSkeletalMeshAsset()->GetName()
    //    : TEXT("NULL"));

    //// 소켓 존재 여부 확인
    //bool bSocketExists = NewOwner->GetMesh()->DoesSocketExist(AttachSocketName);
    //UE_LOG(LogTemp, Warning, TEXT("Socket [%s] exists: %s"),
    //    *AttachSocketName.ToString(),
    //    bSocketExists ? TEXT("YES") : TEXT("NO"));

    AttachToComponent(
        NewOwner->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        AttachSocketName
    );
    UE_LOG(LogTemp, Warning, TEXT("Attached to socket: %s, Location: %s"),
        *AttachSocketName.ToString(),
        *GetActorLocation().ToString());
}

void AWeaponBase::Unequip()
{
    OwnerCharacter = nullptr;
    SetOwner(nullptr);
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AWeaponBase::Interact_Implementation(ACharacter* Interactor)
{
    if (AKangPlayerCharacter* Character = Cast<AKangPlayerCharacter>(Interactor))
    {
        UInventoryComponent* InventoryComponent = Character->FindComponentByClass<UInventoryComponent>();
        if (InventoryComponent)
            InventoryComponent->PickupWeapon(this);
    }
}

FText AWeaponBase::GetInteractHintText_Implementation()
{
    //return FText::FromString(TEXT("E 줍기")); 이거 왜 안 됨
	return FText::FromString(TEXT("E - Pick Up"));
}