// Fill out your copyright notice in the Description page of Project Settings.

#include "MontageHelper.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"

float MontageHelper::Play(USkeletalMeshComponent* Mesh, UAnimMontage* Montage, float PlayRate)
{
	if (!Mesh || !Montage) return 0.f;

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	return AnimInstance ? AnimInstance->Montage_Play(Montage, PlayRate) : 0.f;
}

float MontageHelper::PlayWithEndDelegate(USkeletalMeshComponent* Mesh, UAnimMontage* Montage,
	FOnMontageEnded EndDelegate, float PlayRate)
{
	if (!Mesh || !Montage) return 0.f;

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance) return 0.f;

	const float Duration = AnimInstance->Montage_Play(Montage, PlayRate);
	if (Duration > 0.f)
	{
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
	return Duration;
}
