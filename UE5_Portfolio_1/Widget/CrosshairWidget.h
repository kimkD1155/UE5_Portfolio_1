// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

/**
 * C++ 는 장전 상태/진행률만 매 프레임 밀어넣고, 실제 시각화(크로스헤어 숨김,
 * 원형 프로그레스바 표시/서식)는 BP 하위 클래스가 구현한다 (기존 위젯들과 동일한 관례).
 */
UCLASS()
class UE5_PORTFOLIO_1_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// HUDComponent 가 매 프레임 호출 — 장전 중이 아니면 Progress 는 0.
	UFUNCTION(BlueprintImplementableEvent, Category = "Crosshair")
	void OnReloadProgressUpdated(bool bIsReloading, float Progress);
};
