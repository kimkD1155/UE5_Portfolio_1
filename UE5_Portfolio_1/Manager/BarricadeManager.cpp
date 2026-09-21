// Fill out your copyright notice in the Description page of Project Settings.

#include "BarricadeManager.h"
#include "../Props/Barricade.h"
#include "../Core/KangPlayerState.h"
#include "../Core/UpgradeType.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UBarricadeManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BindToPlayerState();
}

void UBarricadeManager::BindToPlayerState()
{
	if (BoundPlayerState.IsValid()) return;

	AKangPlayerState* PS = nullptr;
	if (const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PS = PC->GetPlayerState<AKangPlayerState>();
	}

	if (!PS)
	{
		GetWorld()->GetTimerManager().SetTimer(BindRetryHandle, this, &UBarricadeManager::BindToPlayerState, 0.25f, false);
		return;
	}

	BoundPlayerState = PS;
	PS->OnUpgradesChanged.AddDynamic(this, &UBarricadeManager::HandleUpgradesChanged);
	HandleUpgradesChanged();
}

void UBarricadeManager::HandleUpgradesChanged()
{
	const AKangPlayerState* PS = BoundPlayerState.Get();
	UpgradeMultiplier = PS ? PS->GetUpgradeMultiplier(EUpgradeType::BarricadeHealth) : 1.f;
	RecomputeMaxHealth();
}

void UBarricadeManager::RecomputeMaxHealth()
{
	const float OldMax = MaxHealth;
	MaxHealth = TotalBaseMaxHealth * UpgradeMultiplier;

	// 최대치만 올리고, 증가분만큼 현재 체력도 같이 채운다 (파괴된 뒤에는 그대로 둔다).
	if (MaxHealth > OldMax && !bIsDestroyed)
	{
		Health = FMath::Min(Health + (MaxHealth - OldMax), MaxHealth);
	}
	else
	{
		Health = FMath::Min(Health, MaxHealth);
	}

	OnHPChanged.Broadcast(Health, MaxHealth);
}

void UBarricadeManager::RegisterBarricade(ABarricade* Barricade, float BaseMaxHealthContribution)
{
	if (!Barricade) return;

	ActiveBarricades.AddUnique(Barricade);
	TotalBaseMaxHealth += FMath::Max(0.f, BaseMaxHealthContribution);
	RecomputeMaxHealth();
}

void UBarricadeManager::UnregisterBarricade(ABarricade* Barricade)
{
	ActiveBarricades.RemoveAll([](const TWeakObjectPtr<ABarricade>& W) { return !W.IsValid(); });
	ActiveBarricades.Remove(Barricade);
	// 개별 조각의 최대체력 기여분은 빼지 않는다 — 실제로 바리케이드가 게임 중 Destroy() 되는
	// 경로가 없어서(파괴되면 시각적으로만 부서짐) 이 함수는 레벨 전환/정리 시점 정도에만 불린다.
}

void UBarricadeManager::ApplyDamage(float Amount, AActor* DamageInstigator)
{
	if (bIsDestroyed || Amount <= 0.f) return;

	Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
	OnHPChanged.Broadcast(Health, MaxHealth);

	if (Health <= 0.f)
	{
		bIsDestroyed = true;

		ActiveBarricades.RemoveAll([](const TWeakObjectPtr<ABarricade>& W) { return !W.IsValid(); });
		for (const TWeakObjectPtr<ABarricade>& WeakBarricade : ActiveBarricades)
		{
			if (ABarricade* Barricade = WeakBarricade.Get())
			{
				Barricade->NotifyDestroyedByManager();
			}
		}

		OnAllDestroyed.Broadcast();
	}
}

void UBarricadeManager::Repair(float Amount)
{
	if (bIsDestroyed || Amount <= 0.f) return;

	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	OnHPChanged.Broadcast(Health, MaxHealth);
}
