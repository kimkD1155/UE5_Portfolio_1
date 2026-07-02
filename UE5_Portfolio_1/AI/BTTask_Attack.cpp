// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Props/Barricade.h"
#include "Kismet/GameplayStatics.h"


UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	// Blackboard에서 타겟 가져오기
	AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
	if (!Target) return EBTNodeResult::Failed;

	// 거리 체크
	float Distance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	if (Distance > AttackRange)
	{
		return EBTNodeResult::Failed; // 범위 밖이면 다시 MoveTo로
	}

	//UE_LOG(LogTemp, Warning, TEXT("Target: %s"), Target ? *Target->GetName() : TEXT("NULL"));
	//UE_LOG(LogTemp, Warning, TEXT("Distance: %.1f / AttackRange: %.1f"), Distance, AttackRange);

	// 데미지 적용
	UGameplayStatics::ApplyDamage(
		Target,
		AttackDamage,
		AIController,
		Pawn,
		nullptr
	);

	UE_LOG(LogTemp, Warning, TEXT("Enemy attacked barricade: %.1f damage"), AttackDamage);

	// 쿨다운 대기 후 Success
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

	OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("TargetActor")); // 추가
	// 쿨다운은 BT 노드 사이에 Wait 노드로 처리하는 게 더 깔끔함
	return EBTNodeResult::Succeeded;
}