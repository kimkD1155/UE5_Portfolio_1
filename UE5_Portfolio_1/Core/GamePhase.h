// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePhase.generated.h"

// 게임 루프의 현재 국면. AKangGameState 가 보유하고 AKangPlayerGameModeBase 가 전이시킨다.
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	// 초기값 — 아직 첫 국면이 시작되지 않음. GameMode 가 BeginPlay 에서 Day/Night 로 바꾼다.
	None,
	// 수색 국면 — 스폰 정지, 낮. 플레이어가 AScavengePoint 를 돌며 코인을 모으고 상점을 이용한다.
	Day,
	// 전투 국면 — 스포너 가동, 밤. 좀비 웨이브가 바리케이드를 공격한다.
	Night,
	// 종료 — 바리케이드 전멸 또는 플레이어 사망. 모든 타이머 정지.
	GameOver
};
