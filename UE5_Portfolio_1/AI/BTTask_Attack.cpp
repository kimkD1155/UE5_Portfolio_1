// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../Props/Barricade.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/EnemyCharacter.h"

#include "DrawDebugHelpers.h"


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

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Pawn);
	if (!Enemy) return EBTNodeResult::Failed;

	// 거리 체크
	// 수정 - 컴포넌트 바운드 기준 가장 가까운 점
	FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("TargetLocation"));
	float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetLocation);




//#if WITH_EDITOR
//// 적 → 타겟 가장 가까운 점 라인
//	DrawDebugLine(GetWorld(), Pawn->GetActorLocation(), TargetLocation, FColor::Yellow, false, 1.f, 0, 2.f);
//	// 타겟 가장 가까운 점 구체
//	DrawDebugSphere(GetWorld(), TargetLocation, 20.f, 12, FColor::Yellow, false, 1.f);
//	// AttackRange 구체
//	DrawDebugSphere(GetWorld(), Pawn->GetActorLocation(), Enemy->GetAttackRange(), 24,
//		Distance <= Enemy->GetAttackRange() ? FColor::Green : FColor::Red, false, 1.f);
//#endif

	if (Distance > Enemy->GetAttackRange())
	{
		
		return EBTNodeResult::Failed; // 범위 밖이면 다시 MoveTo로
	}

	FBTAttackMemory* Memory = reinterpret_cast<FBTAttackMemory*>(NodeMemory);
	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	TWeakObjectPtr<AEnemyCharacter> WeakEnemy = Enemy;

	Memory->EndDelegateHandle = Enemy->OnAttackMontageEnded.AddLambda([this, WeakOwnerComp, WeakEnemy, Memory]()
		{
			if (WeakEnemy.IsValid())
			{
				WeakEnemy->OnAttackMontageEnded.Remove(Memory->EndDelegateHandle);
			}
			if (WeakOwnerComp.IsValid())
			{
				FinishLatentTask(*WeakOwnerComp, EBTNodeResult::Succeeded);
			}
		});

	Enemy->PlayAttackMontage();

	return EBTNodeResult::InProgress;

	//UE_LOG(LogTemp, Warning, TEXT("Target: %s"), Target ? *Target->GetName() : TEXT("NULL"));
	//UE_LOG(LogTemp, Warning, TEXT("Distance: %.1f / AttackRange: %.1f"), Distance, AttackRange);

	// 데미지 적용
	//Enemy->PlayAttackMontage();
	//

	//// 쿨다운 대기 후 Success
	//FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

	//OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("TargetActor")); // 추가
	//// 쿨다운은 BT 노드 사이에 Wait 노드로 처리하는 게 더 깔끔함
	//return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AEnemyCharacter* Enemy = AIController ? Cast<AEnemyCharacter>(AIController->GetPawn()) : nullptr)
	{
		FBTAttackMemory* Memory = reinterpret_cast<FBTAttackMemory*>(NodeMemory);
		Enemy->OnAttackMontageEnded.Remove(Memory->EndDelegateHandle);
	}
	return EBTNodeResult::Aborted;
}