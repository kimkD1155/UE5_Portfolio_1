// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "GameFramework/Character.h"
// OwnerCharacter->GetController() 같은 멤버함수를 호출하려면 반드시 완전한 타입 정의가 필요
#include "WeaponAnimSet.h"
#include "WeaponSlot.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Pistol,
	Rifle
};


UCLASS()
class UE5_PORTFOLIO_1_API AWeaponBase : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// 소유 캐릭터
	UPROPERTY()
	ACharacter* OwnerCharacter;

protected:

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

public:
	UFUNCTION()
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::None;

	// 이 무기가 들어갈 인벤토리 슬롯. 인벤토리가 무기 타입을 분기하지 않도록 무기가 직접 명시.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponSlot PreferredSlot = EWeaponSlot::Primary;

	UFUNCTION(BlueprintPure)
	EWeaponSlot GetPreferredSlot() const { return PreferredSlot; }



protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FText WeaponName = FText::FromString(TEXT("Unknown"));

public:
	// ── 장착 / 해제 ───────────────────────────────
	void Equip(ACharacter* NewOwner);
	void Unequip();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FTransform GripOffset;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName GripSocketName = "Hand_r_socket";

	// 조준(ADS) 시 이 소켓이 AimMount(카메라 앞의 조준 정렬 목표) 위치/회전에 오도록
	// 무기 자체를 끌어와 맞춘다 (AKangPlayerCharacter::UpdateWeaponPose 참고). 소켓의
	// "정면(+X)"이 총구 방향이 아니라 반대(사수 쪽)를 향하도록 잘못 잡혀 있으면 무기
	// 몸체 전체가 반대로 돌아가버리니 소켓 Rotation의 Yaw를 180도 뒤집어야 한다.
	// 소켓이 없는 무기는 조준해도 힙파이어 위치 그대로 유지된다.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName ScopeSocketName = "ScopeSocket";

	// 스켈레톤 에디터에서 ScopeSocket 회전을 직접 고치는 대신, 여기서 빠르게 보정해가며
	// 테스트할 수 있도록 둔다. 소켓이 반대 방향을 보고 있다면 Yaw=180 부터 시도해볼 것.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FRotator ScopeSocketRotationCorrection;

	// ── 자식 클래스에서 override ───────────────────
	// IInteractableInterface
	virtual void Interact_Implementation(ACharacter* Interactor);
	virtual FText GetInteractHintText_Implementation();

	UFUNCTION(BlueprintPure)
	EWeaponType GetWeaponType() const { return WeaponType; }

	UFUNCTION(BlueprintPure)
	FText GetWeaponName() const { return WeaponName; }


	virtual void StartFire() {}
	virtual void StopFire() {}

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	// ── 무기 메시(1인칭 총기 애님)에서 재생되는 몽타주 ──
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Weapon")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Weapon")
	UAnimMontage* ReloadMontage;

	// ── 소유 캐릭터 스켈레톤에서 재생되는 몽타주 세트 (무기별 DataAsset) ──
	// 캐릭터가 switch(EWeaponType) 로 고르던 로직을 데이터로 분리 (OCP).
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Character")
	TObjectPtr<UWeaponAnimSet> CharacterAnimSet;

	UFUNCTION(BlueprintPure, Category = "Animation")
	UAnimMontage* GetCharacterFireMontage() const { return CharacterAnimSet ? CharacterAnimSet->FireMontage : nullptr; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	UAnimMontage* GetCharacterReloadMontage() const { return CharacterAnimSet ? CharacterAnimSet->ReloadMontage : nullptr; }

	UFUNCTION(BlueprintPure, Category = "Animation")
	UAnimMontage* GetCharacterEquipMontage() const { return CharacterAnimSet ? CharacterAnimSet->EquipMontage : nullptr; }

	void PlayFireSound();
	void PlayFireMontage();
	void PlayReloadMontage();

	UPROPERTY(BlueprintAssignable)
	FOnWeaponFired OnWeaponFired;



};
