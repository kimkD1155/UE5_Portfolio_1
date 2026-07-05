// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ReloadFinished.h"
#include "../Animation/KangAnimInstance.h"

void UAnimNotify_ReloadFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	UKangAnimInstance* AnimInst = Cast<UKangAnimInstance>(MeshComp->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->OnReloadFinishedDelegate.Broadcast();
	}
}
