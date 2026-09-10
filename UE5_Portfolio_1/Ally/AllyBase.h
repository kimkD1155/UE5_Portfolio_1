// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/WeaponHolder.h"
#include "AllyBase.generated.h"

class AWeaponBase;
class USphereComponent;
class UHealthComponent;
class UCombatComponent;

UCLASS()
class UE5_PORTFOLIO_1_API AAllyBase : public ACharacter, public IWeaponHolder
{
	GENERATED_BODY()

public:
	AAllyBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// IWeaponHolder
	virtual AWeaponBase* GetActiveWeapon() const override { return EquippedWeapon; }
	// 아군 공격력 업그레이드 배율 (플레이어의 런 진행도에서 조회)
	virtual float GetOutgoingDamageMultiplier() const override;

	//────────────────────────── 컴포넌트 ──────────────────────────
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComponent;

	UFUNCTION()
	void HandleDeath(AActor* DamageInstigator);

	//────────────────────────── 적 감지 관련 ──────────────────────────
protected:
	// 적 감지용 구체 콜리전 (이 범위 안에 들어온 적만 타겟팅 대상)
	UPROPERTY(EditDefaultsOnly)
	USphereComponent* AttackRangeSphere;
	// 감지 반경
	UPROPERTY(EditDefaultsOnly)
	float AttackRange{ 800.f };
	// 현재 범위 안에 들어와 있는 적 목록
	UPROPERTY()
	TArray<AActor*> EnemiesInRange;
	// 현재 공격 대상으로 선택된 적
	UPROPERTY()
	AActor* CurrentTarget;

	AActor* FindTarget(); // 지금은 가장 가까운 적만

	UFUNCTION()
	void OnEnemyEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEnemyExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//────────────────────────── 전투 ──────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;
	// 장착된 무기
	UPROPERTY()
	AWeaponBase* EquippedWeapon;
	void EquipDefaultWeapon();

	void Attack(); // AttackTimer 로 주기 호출

	UPROPERTY()
	FTimerHandle AttackTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackInterval = 1.f;

	//────────────────────────── 디버그 ──────────────────────────
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowAttackRangeDebug = true;
};
