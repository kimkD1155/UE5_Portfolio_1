// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultWidget.generated.h"

/**
 * 게임오버 화면. "DAY N 에서 전멸" + 재시작 버튼.
 * 표시는 BP, 재시작 로직은 C++.
 */
UCLASS(Abstract)
class UE5_PORTFOLIO_1_API UResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 게임오버 시 HUDComponent 가 호출. DayReached = 버틴 일수.
	UFUNCTION(BlueprintImplementableEvent, Category = "Result")
	void OnResultShown(int32 DayReached);

	// 재시작 버튼에서 호출 — 현재 레벨을 다시 연다.
	UFUNCTION(BlueprintCallable, Category = "Result")
	void RestartGame();
};
