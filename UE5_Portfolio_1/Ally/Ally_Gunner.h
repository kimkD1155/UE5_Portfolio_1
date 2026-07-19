// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AllyBase.h"
#include "Ally_Gunner.generated.h"

/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API AAlly_Gunner : public AAllyBase
{
	GENERATED_BODY()
	
protected:
	virtual void Attack() override;
};
