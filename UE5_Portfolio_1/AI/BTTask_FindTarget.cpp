// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"


UBTTask_FindTarget::UBTTask_FindTarget()
{
	NodeName = TEXT("Find Target");
}

EBTNodeResult::Type UBTTask_FindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;

	if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName) != nullptr)
	{
		return EBTNodeResult::Succeeded;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetTag, FoundActors);
	if (FoundActors.Num() == 0) return EBTNodeResult::Failed;

	// 가장 가까운 방어벽 찾기
	AActor* ClosestWall = nullptr;
	float ClosestDist = TNumericLimits<float>::Max();
	const FVector MyLocation = AIC->GetPawn()->GetActorLocation();

	for (AActor* Actor : FoundActors)
	{
		if (!Actor) continue;
		float Dist = FVector::DistSquared(MyLocation, Actor->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			ClosestWall = Actor;
		}
	}

	if (!ClosestWall) return EBTNodeResult::Failed;

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestWall);

	// 바리케이드 바운딩 박스 기준 가장 가까운 점 계산
	FBox BoundingBox = ClosestWall->GetComponentsBoundingBox();
	FVector ClosestPoint = BoundingBox.GetClosestPointTo(MyLocation);
	ClosestPoint.Z = MyLocation.Z; // Z축 고정

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, ClosestPoint);

	return EBTNodeResult::Succeeded;
}