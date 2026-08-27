// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_EnableAttackHitBox.h"
#include "../../Character/EnemyCharacter.h"

void UAnimNotify_EnableAttackHitBox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();

	if (!Owner) return;

	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(Owner);

	if (!EnemyCharacter) return;

	EnemyCharacter->EnableAttackHitBox();

}