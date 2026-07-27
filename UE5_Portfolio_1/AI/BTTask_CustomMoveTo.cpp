// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CustomMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Character/EnemyCharacter.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_CustomMoveTo::UBTTask_CustomMoveTo()
{
	NodeName = TEXT("Custom Move To");
}

EBTNodeResult::Type UBTTask_CustomMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(AIC->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!Target) return EBTNodeResult::Failed;


	// AcceptableRadius = AttackRange
	AcceptableRadius = Enemy->GetAttackRange() - tolerance;

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
