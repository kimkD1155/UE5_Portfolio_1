// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveForward.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UBTTask_MoveForward : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveForward();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category = "Move")
	float AttackRange = 150.f;
};
