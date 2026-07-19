// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KangPlayerController.generated.h"

class UInputMappingContext;
class AAllyBase;

UCLASS()
class UE5_PORTFOLIO_1_API AKangPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;



	void UpdateGhostPreview();

	UPROPERTY()
	AAllyBase* GhostPreview;

	TSubclassOf<AAllyBase> PendingAllyClass;

	bool bIsInPlacementMode = false;

public:
	// 배치 모드 시작 (상점에서 구매 시 호출)
	UFUNCTION(BlueprintCallable)
	void StartPlacementMode(TSubclassOf<AAllyBase> AllyClass);

	// 배치 확정
	void ConfirmPlacement();

	// 배치 취소
	void CancelPlacement();

	bool IsInPlacementMode() const { return bIsInPlacementMode; }

};
