// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MoveForward.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveForward::UBTTask_MoveForward()
{
	NodeName = TEXT("Move Forward");
}

EBTNodeResult::Type UBTTask_MoveForward::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!Target) return EBTNodeResult::Failed;

	// 바운딩 박스 기준 가장 가까운 점으로 이동
	FBox BoundingBox = Target->GetComponentsBoundingBox();
	FVector ClosestPoint = BoundingBox.GetClosestPointTo(Pawn->GetActorLocation());
	ClosestPoint.Z = Pawn->GetActorLocation().Z; // Z축은 적 높이로 고정

	AIC->MoveToLocation(ClosestPoint, AttackRange);

	return EBTNodeResult::Succeeded;
}
