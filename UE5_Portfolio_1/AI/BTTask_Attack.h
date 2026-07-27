// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Attack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	// 한 번 공격 시 데미지
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 30.f;

	// 공격 후 대기 시간 (BT가 루프 돌 때 딜레이 역할)
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackCooldown = 1.0f;
};
