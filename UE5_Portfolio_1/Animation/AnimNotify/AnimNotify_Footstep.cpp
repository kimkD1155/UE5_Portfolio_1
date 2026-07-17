// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_Footstep.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!FootstepSound || !MeshComp) return;

	UGameplayStatics::PlaySoundAtLocation(MeshComp, FootstepSound, MeshComp->GetComponentLocation());
}
