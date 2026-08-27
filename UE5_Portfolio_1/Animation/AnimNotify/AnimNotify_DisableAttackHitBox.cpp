// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_DisableAttackHitBox.h"
#include "../../Character/EnemyCharacter.h"

void UAnimNotify_DisableAttackHitBox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(Owner);
	if (!EnemyCharacter) return;
	EnemyCharacter->DisableAttackHitBox();
}