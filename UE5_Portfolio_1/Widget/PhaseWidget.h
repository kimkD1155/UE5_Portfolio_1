// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Core/GamePhase.h"
#include "PhaseWidget.generated.h"

/**
 * 화면 상단의 국면/일차/타이머/잔여 좀비 표시.
 * C++ 는 데이터만 밀어넣고, 실제 레이아웃과 서식은 BP 하위 클래스가 구현한다.
 * (기존 위젯들처럼 C++ 베이스 + BP 디자인 분리.)
 */
UCLASS(Abstract)
class UE5_PORTFOLIO_1_API UPhaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 국면 또는 일차가 바뀔 때 HUDComponent 가 호출
	UFUNCTION(BlueprintImplementableEvent, Category = "Phase")
	void OnPhaseUpdated(EGamePhase Phase, int32 DayNumber);

	// 잔여 시간이 갱신될 때 (약 1초 간격)
	UFUNCTION(BlueprintImplementableEvent, Category = "Phase")
	void OnTimeUpdated(float SecondsRemaining);

	// 밤 동안 남은 좀비 수가 바뀔 때
	UFUNCTION(BlueprintImplementableEvent, Category = "Phase")
	void OnEnemiesLeftUpdated(int32 Count);
};
