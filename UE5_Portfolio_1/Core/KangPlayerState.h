// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "KangPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinChanged, int32, CurrentCoin);
/**
 * 
 */
UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnCoinChanged OnCoinChanged;

	UFUNCTION(BlueprintCallable)
	void AddCoin(int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool SpendCoin(int32 Amount);

	UFUNCTION(BlueprintPure)
	int32 GetCoin() const { return Coin; }

private:
	UPROPERTY(VisibleAnywhere)
	int32 Coin = 100;
};
