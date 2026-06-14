// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KangPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC;

	virtual void BeginPlay() override;
};
