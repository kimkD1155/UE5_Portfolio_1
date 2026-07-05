// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	

	if (!BehaviorTreeAsset) return;

	UBlackboardComponent* BlackboardComp = nullptr;
	if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp))
	{
		Blackboard = BlackboardComp;
		RunBehaviorTree(BehaviorTreeAsset);
	}
}