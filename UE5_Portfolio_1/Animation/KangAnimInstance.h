// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KangAnimInstance.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadFinished);
/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API UKangAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnReloadFinished OnReloadFinishedDelegate;
};
