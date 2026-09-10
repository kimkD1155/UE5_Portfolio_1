// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 체력이 변할 때마다 브로드캐스트 (현재/최대/변화량/가해자)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, float, Health, float, MaxHealth, float, Delta, AActor*, DamageInstigator);
// 체력이 0이 되는 순간 한 번 브로드캐스트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DamageInstigator);

/**
 * 액터의 체력/피격/사망을 담당하는 재사용 컴포넌트.
 * Enemy, Barricade, Player, Ally 등 "죽을 수 있는" 모든 액터가 공유한다.
 * 소유 액터의 OnTakeAnyDamage 에 자동으로 바인딩되므로 소유자는 델리게이트만 구독하면 된다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UE5_PORTFOLIO_1_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	// 직접 데미지/회복을 가하고 싶을 때 (OnTakeAnyDamage 경로를 타지 않는 경우)
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Amount, AActor* DamageInstigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	// 초당 재생량을 런타임에 설정한다 (체력 재생 업그레이드). 0 이하면 재생을 끈다.
	// 값이 양수면 bRegenEnabled 도 자동으로 켜진다.
	UFUNCTION(BlueprintCallable, Category = "Health|Regen")
	void SetRegenPerSecond(float NewRate);

	// 생성자 또는 스폰 직후에 최대 체력을 설정 (기본적으로 현재 체력도 가득 채움)
	void SetMaxHealth(float NewMaxHealth, bool bFillToMax = true);

	// OnTakeAnyDamage 자동 바인딩 여부를 소유자가 결정한다.
	// 부위별 데미지 배율처럼 소유자가 직접 데미지를 해석해야 하는 경우 false 로 끄고
	// ApplyDamage() 를 직접 호출한다. 반드시 컴포넌트 BeginPlay 이전(생성자)에서 호출할 것.
	void SetBindToOwnerDamage(bool bInBind) { bBindToOwnerDamage = bInBind; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsFullHealth() const { return Health >= MaxHealth; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	// true 면 BeginPlay 에서 소유 액터의 OnTakeAnyDamage 에 자동 바인딩한다.
	UPROPERTY(EditAnywhere, Category = "Health")
	bool bBindToOwnerDamage = true;

	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	float Health = 0.f;

	// ── 체력 재생 (기본 꺼짐. 플레이어가 업그레이드로 켠다) ──────────
	UPROPERTY(EditAnywhere, Category = "Health|Regen")
	bool bRegenEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Health|Regen", meta = (ClampMin = "0.0"))
	float RegenPerSecond = 0.f;

	// 마지막 피격 후 이 시간이 지나야 재생을 시작한다
	UPROPERTY(EditAnywhere, Category = "Health|Regen", meta = (ClampMin = "0.0"))
	float RegenStartDelay = 4.f;

	UPROPERTY(EditAnywhere, Category = "Health|Regen", meta = (ClampMin = "0.05"))
	float RegenTickInterval = 0.5f;

private:
	bool bIsDead = false;

	// 체력 변경의 단일 통로: 클램프된 값 반영 + 델리게이트 브로드캐스트 + 사망 처리
	void SetHealth(float NewHealth, AActor* DamageInstigator);

	// 재생 타이머 (Tick 대신 타이머 사용)
	FTimerHandle RegenDelayTimerHandle;
	FTimerHandle RegenTickTimerHandle;
	void ScheduleRegenAfterDelay();
	void BeginRegenTicking();
	void RegenTick();
	void StopRegen();
};
