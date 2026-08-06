// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "GameFramework/Character.h"
// OwnerCharacter->GetController() 같은 멤버함수를 호출하려면 반드시 완전한 타입 정의가 필요
#include "WeaponBase.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Pistol,
	Rifle,
	Melee
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
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::None;

	// 캐릭터 손 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName AttachSocketName = TEXT("hand_r_Socket");

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FText WeaponName = FText::FromString(TEXT("Unknown"));

public:
	// ── 장착 / 해제 ───────────────────────────────
	void Equip(ACharacter* NewOwner);
	void Unequip();

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
	virtual void StartAim() {}
	virtual void StopAim() {}

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* FireSound;

	void PlayFireSound();


};
