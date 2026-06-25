// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_FindTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"


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
	// 테스트용: 직접 MoveTo 호출해서 결과 확인
	EPathFollowingRequestResult::Type Result = AIC->MoveToActor(ClosestWall, 50.f);
	UE_LOG(LogTemp, Warning, TEXT("MoveToActor Result: %d"), (int32)Result);
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	
	
	return EBTNodeResult::Succeeded;


}