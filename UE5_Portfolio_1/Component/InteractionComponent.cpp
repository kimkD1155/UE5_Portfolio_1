// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "GameFramework/Character.h"
#include "../Weapon/WeaponBase.h"
#include "HUDComponent.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	HUDComp = OwnerCharacter->FindComponentByClass<UHUDComponent>();
}


// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateInteractionTarget();
}

void UInteractionComponent::UpdateInteractionTarget()
{
	// 카메라 위치 / 방향 가져오기
	if (!OwnerCharacter || !OwnerCharacter->GetController()) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + CameraRotation.Vector() * InteractTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, Params
	);

	//// 맞은 게 있으면 초록색, 없으면 빨간색
	//DrawDebugLine(
	//	GetWorld(),
	//	TraceStart,
	//	bHit ? HitResult.ImpactPoint : TraceEnd,
	//	bHit ? FColor::Green : FColor::Red,
	//	false,    // 지속 표시 여부 (false = 매 프레임 갱신)
	//	-1.f,     // 지속 시간 (-1 = 1프레임만)
	//	0,
	//	1.5f      // 선 두께
	//);

	AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;

	// IInteractable 구현한 액터만 대상으로
	if (HitActor && HitActor->Implements<UInteractableInterface>())
	{
		if (CurrentInteractTarget != HitActor)
		{
			CurrentInteractTarget = HitActor;
			// TODO: Step 4에서 HUD 힌트 텍스트 업데이트
			if (HUDComp)
			{

				FText HintText = IInteractableInterface::Execute_GetInteractHintText(HitActor);
				HUDComp->ShowInteractHint(HintText);
			}
		}
	}
	else
	{
		if (CurrentInteractTarget != nullptr)
		{
			CurrentInteractTarget = nullptr;
			// TODO: Step 4에서 HUD 힌트 숨기기
			if (HUDComp)
			{
				UE_LOG(LogTemp, Warning, TEXT("Hiding Interact Hint"));
				HUDComp->HideInteractHint();
			}
		}
		
	}
	// 현재 타겟액터가 있는지 확인
}


void UInteractionComponent::PickupWeapon(AWeaponBase* Weapon)
{
	if (!Weapon) return;

	if (EquippedWeapon)
		DropWeapon();

	EquippedWeapon = Weapon;
	EquippedWeapon->Equip(OwnerCharacter);
}



void UInteractionComponent::DropWeapon()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->Unequip();

	FVector DropLocation = OwnerCharacter->GetActorLocation()
		+ OwnerCharacter->GetActorForwardVector() * 100.f
		+ FVector(0.f, 0.f, -50.f);
	EquippedWeapon->SetActorLocation(DropLocation);
	EquippedWeapon = nullptr;
}