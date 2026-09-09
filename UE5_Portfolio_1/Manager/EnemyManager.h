// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyManager.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyCountChanged, int32 /*NewCount*/);

UCLASS()
class UE5_PORTFOLIO_1_API UEnemyManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// 스포너가 적을 스폰한 직후 호출
	void RegisterEnemy(ACharacter* Enemy);

	// 적이 죽었을 때 호출 (직접 호출 or OnDestroyed 바인딩)
	UFUNCTION()
	void UnregisterEnemy(AActor* Enemy);

	int32 GetEnemyCount() const { return ActiveEnemies.Num(); }

	// UI(HUDComponent) 등에서 카운트 변화 구독용
	FOnEnemyCountChanged OnEnemyCountChanged;

private:
	UPROPERTY()
	TArray<ACharacter*> ActiveEnemies;
};
