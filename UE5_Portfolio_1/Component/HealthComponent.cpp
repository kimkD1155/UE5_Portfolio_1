// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

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

	OnHealthChanged.Broadcast(Health, MaxHealth, Delta, DamageInstigator);

	if (bJustDied)
	{
		OnDeath.Broadcast(DamageInstigator);
	}
}
