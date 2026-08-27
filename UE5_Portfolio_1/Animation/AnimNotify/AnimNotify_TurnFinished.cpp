// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_TurnFinished.h"
#include "../../Character/KangPlayerCharacter.h"

void UAnimNotify_TurnFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	AKangPlayerCharacter* PlayerCharacter = Cast<AKangPlayerCharacter>(MeshComp->GetOwner());
	if (PlayerCharacter)
	{
		//PlayerCharacter->SetTurnDirection(ETurnDirection::None);
	}
}

