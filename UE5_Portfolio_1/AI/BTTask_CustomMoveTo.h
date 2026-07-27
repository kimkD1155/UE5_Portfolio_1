// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_CustomMoveTo.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UBTTask_CustomMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
public:
	UBTTask_CustomMoveTo();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	float tolerance = 50.f;
	// 캡슐 등의 크기로 인해 공격사거리보다 목표로부터의 적 위치가 크게 나와 허용 오차 범위를 설정함

};
