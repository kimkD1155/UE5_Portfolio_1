// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;
class ARangedWeapon;
class ACharacter;
class USkeletalMeshComponent;
class UAnimMontage;

/**
 * "장착한 원거리 무기를 사용한다" 는 행동을 캡슐화한 재사용 컴포넌트.
 * 플레이어(입력)와 아군(AI)이 동일하게 사용한다.
 *
 * 제어할 무기는 소유 액터가 구현한 IWeaponHolder 로 조회하므로
 * 이 컴포넌트는 소유자의 구체 타입을 알지 못한다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE5_PORTFOLIO_1_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	// 외부(캐릭터 입력 또는 AI)가 호출하는 진입점
	void StartFire();       // 플레이어: 버튼 누름 (자동/반자동은 무기가 결정)
	void StopFire();        // 플레이어: 버튼 뗌
	void FireOnce();        // AI: 자동/반자동 무관하게 1발
	void StartAim();
	void StopAim();
	void Reload();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsReloading() const { return ReloadingWeapon != nullptr; }

protected:
	virtual void BeginPlay() override;

	AWeaponBase* GetActiveWeapon() const;
	ARangedWeapon* GetActiveRangedWeapon() const;
	USkeletalMeshComponent* GetCharacterMesh() const;

	UFUNCTION()
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 캐릭터 재장전 몽타주가 없을 때 ReloadTime 경과 후 장전 완료
	void FinishReloadByTimer();

	// 무기가 발사될 때 캐릭터 사격 몽타주 재생
	UFUNCTION()
	void HandleWeaponFired();

	// 장전 완료/중단 공통 처리
	void CompleteReload(bool bInterrupted);

private:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	// 현재 리로드 중인 무기 (완료 시점에 ReloadFinished 호출 대상)
	UPROPERTY()
	TObjectPtr<ARangedWeapon> ReloadingWeapon;

	FTimerHandle ReloadTimerHandle;
};
