// Fill out your copyright notice in the Description page of Project Settings.


#include "ResultWidget.h"
#include "Kismet/GameplayStatics.h"

void UResultWidget::RestartGame()
{
	// 현재 레벨을 다시 로드 → 모든 상태 초기화
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}
