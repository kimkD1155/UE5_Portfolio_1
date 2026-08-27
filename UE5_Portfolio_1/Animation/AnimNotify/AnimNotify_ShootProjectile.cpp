// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ShootProjectile.h"
#include "../../Enemy/ZombieRanged.h"
#include "../../Enemy/EnemyProjectile.h"

void UAnimNotify_ShootProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AZombieRanged* Enemy = MeshComp ? Cast<AZombieRanged>(MeshComp->GetOwner()) : nullptr)
	{
		Enemy->ShootProjectile();
	}
}