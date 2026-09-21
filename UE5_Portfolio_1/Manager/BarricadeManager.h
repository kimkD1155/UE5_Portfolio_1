// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BarricadeManager.generated.h"

class ABarricade;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSharedBarricadeHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllBarricadesDestroyed);

/**
 * 레벨에 배치된 여러 ABarricade(서로 다른 메시라도)를 "하나의 방어선"으로 묶어 체력을 공유한다.
 * 각 바리케이드는 더 이상 자기 체력을 갖지 않고, 피해/수리를 전부 이 매니저의 공유 풀로 전달한다.
 * 공유 풀이 0이 되면 등록된 바리케이드 전부가 동시에 파괴 처리된다.
 *
 * 최대 체력은 등록된 각 바리케이드의 BaseMaxHealth 합으로 정해진다 — 조각을 더 놓을수록
 * 방어선 전체 체력이 늘어나는 셈이라, 별도의 "공유 최대체력" 설정값을 새로 만들 필요가 없다.
 * (EnemyManager/AllyManager 와 같은 WorldSubsystem 등록 패턴.)
 */
UCLASS()
class UE5_PORTFOLIO_1_API UBarricadeManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UPROPERTY(BlueprintAssignable, Category = "Barricade")
	FOnSharedBarricadeHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Barricade")
	FOnAllBarricadesDestroyed OnAllDestroyed;

	// 바리케이드가 BeginPlay 에서 호출 — 자기 몫의 최대 체력을 공유 풀에 더한다.
	void RegisterBarricade(ABarricade* Barricade, float BaseMaxHealthContribution);
	void UnregisterBarricade(ABarricade* Barricade);

	// 좀비의 공격이 가한 피해를 공유 풀에 적용한다.
	void ApplyDamage(float Amount, AActor* DamageInstigator);
	// E 키 상호작용 등으로 공유 풀을 회복시킨다.
	void Repair(float Amount);

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsDestroyed() const { return bIsDestroyed; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsFullHealth() const { return !bIsDestroyed && Health >= MaxHealth; }

private:
	// PlayerState 가 아직 없을 수 있어 재시도한다 (ABarricade 가 예전에 각자 하던 걸 매니저 하나로 통합).
	void BindToPlayerState();
	UFUNCTION()
	void HandleUpgradesChanged();

	void RecomputeMaxHealth();

	UPROPERTY()
	TArray<TWeakObjectPtr<ABarricade>> ActiveBarricades;

	UPROPERTY()
	TWeakObjectPtr<class AKangPlayerState> BoundPlayerState;

	FTimerHandle BindRetryHandle;

	float TotalBaseMaxHealth = 0.f;
	float UpgradeMultiplier = 1.f;
	float Health = 0.f;
	float MaxHealth = 0.f;
	bool bIsDestroyed = false;
};
