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
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	// ── 핵심: 타겟 위치를 NavMesh 위로 투영해서 가장 가까운 유효 지점 찾기 ──
	//UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	//if (NavSys)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("NavSys is not null"));
	//	FNavLocation ProjectedLocation;
	//	// 큰 범위로 검색 (콜리전 구멍보다 충분히 크게)
	//	FVector QueryExtent(500.f, 500.f, 500.f);

	//	if (NavSys->ProjectPointToNavigation(ClosestWall->GetActorLocation(), ProjectedLocation, QueryExtent))
	//	{
	//		AIC->MoveToLocation(ProjectedLocation.Location, 50.f);
	//		EPathFollowingRequestResult::Type MoveResult = AIC->MoveToLocation(ProjectedLocation.Location, 50.f);
	//		UE_LOG(LogTemp, Warning, TEXT("[%s] MoveToLocation Result: %d, ProjectedLoc: %s"),
	//			*AIC->GetPawn()->GetName(),
	//			(int32)MoveResult,
	//			*ProjectedLocation.Location.ToString());
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Failed to project wall location to NavMesh"));
	//		return EBTNodeResult::Failed;
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("NavSys is null"));
	//}

	return EBTNodeResult::Succeeded;


}