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

	// 랜덤 오프셋 적용
	FVector RandomOffset = FMath::VRand();
	RandomOffset.Z = 0.f;
	RandomOffset.Normalize();
	RandomOffset *= FMath::RandRange(200.f, 500.f);

	FVector TargetLocation = ClosestWall->GetActorLocation() + RandomOffset;
	
	UE_LOG(LogTemp, Warning, TEXT("TargetLocation: %s"), *TargetLocation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("TargetLocationKey: %s"), *TargetLocationKey.SelectedKeyName.ToString());
	
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);

	return EBTNodeResult::Succeeded;


}