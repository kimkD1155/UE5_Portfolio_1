// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Health = MaxHealth; // BeginPlay 전에 값을 읽어도 안전하도록 초기화
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (bBindToOwnerDamage)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			OwnerActor->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		}
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth, bool bFillToMax)
{
	MaxHealth = FMath::Max(1.f, NewMaxHealth);
	Health = bFillToMax ? MaxHealth : FMath::Min(Health, MaxHealth);
}

void UHealthComponent::HandleTakeAnyDamage(AActor* /*DamagedActor*/, float Damage, const UDamageType* /*DamageType*/,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f) return;

	AActor* Instigator = DamageCauser;
	if (!Instigator && InstigatedBy)
	{
		Instigator = InstigatedBy->GetPawn();
	}
	ApplyDamage(Damage, Instigator);
}

void UHealthComponent::ApplyDamage(float Amount, AActor* DamageInstigator)
{
	if (bIsDead || Amount <= 0.f) return;
	SetHealth(FMath::Clamp(Health - Amount, 0.f, MaxHealth), DamageInstigator);
}

void UHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.f) return;
	SetHealth(FMath::Clamp(Health + Amount, 0.f, MaxHealth), nullptr);
}

void UHealthComponent::SetRegenPerSecond(float NewRate)
{
	RegenPerSecond = FMath::Max(0.f, NewRate);
	bRegenEnabled = RegenPerSecond > 0.f;

	if (!bRegenEnabled)
	{
		StopRegen();
	}
	else if (!bIsDead && Health < MaxHealth)
	{
		// 방금 업그레이드로 켠 경우: 대기 없이 바로 재생 시작
		BeginRegenTicking();
	}
}

void UHealthComponent::ScheduleRegenAfterDelay()
{
	StopRegen();
	if (!bRegenEnabled || bIsDead || !GetWorld()) return;

	GetWorld()->GetTimerManager().SetTimer(RegenDelayTimerHandle, this,
		&UHealthComponent::BeginRegenTicking, FMath::Max(RegenStartDelay, 0.01f), false);
}

void UHealthComponent::BeginRegenTicking()
{
	if (!bRegenEnabled || bIsDead || !GetWorld()) return;
	if (Health >= MaxHealth) return;

	GetWorld()->GetTimerManager().SetTimer(RegenTickTimerHandle, this,
		&UHealthComponent::RegenTick, FMath::Max(RegenTickInterval, 0.05f), true);
}

void UHealthComponent::RegenTick()
{
	if (bIsDead)
	{
		StopRegen();
		return;
	}

	Heal(RegenPerSecond * RegenTickInterval);

	if (Health >= MaxHealth)
	{
		StopRegen();
	}
}

void UHealthComponent::StopRegen()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenDelayTimerHandle);
		World->GetTimerManager().ClearTimer(RegenTickTimerHandle);
	}
}

void UHealthComponent::SetHealth(float NewHealth, AActor* DamageInstigator)
{
	const float Delta = NewHealth - Health;
	if (FMath::IsNearlyZero(Delta)) return;

	Health = NewHealth;

	// 사망 여부를 브로드캐스트 전에 확정해서, OnHealthChanged 구독자도 일관된 상태를 본다.
	const bool bJustDied = (Health <= 0.f && !bIsDead);
	if (bJustDied)
	{
		bIsDead = true;
	}

	// 피해를 입으면 재생을 멈추고 지연 후 재개, 사망 시 완전 정지
	if (bJustDied)
	{
		StopRegen();
	}
	else if (Delta < 0.f)
	{
		ScheduleRegenAfterDelay();
	}

	OnHealthChanged.Broadcast(Health, MaxHealth, Delta, DamageInstigator);

	if (bJustDied)
	{
		OnDeath.Broadcast(DamageInstigator);
	}
}
