// Fill out your copyright notice in the Description page of Project Settings.


#include "KangGameState.h"

void AKangGameState::SetPhase(EGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	LastBroadcastSecond = -1; // 다음 UpdatePhaseTime 에서 강제로 한 번 브로드캐스트
	OnPhaseChanged.Broadcast(CurrentPhase);
}

void AKangGameState::UpdatePhaseTime(float NewRemaining)
{
	PhaseTimeRemaining = FMath::Max(0.f, NewRemaining);

	const int32 Second = FMath::CeilToInt(PhaseTimeRemaining);
	if (Second != LastBroadcastSecond)
	{
		LastBroadcastSecond = Second;
		OnPhaseTimeChanged.Broadcast(PhaseTimeRemaining);
	}
}

void AKangGameState::SetSpawningActive(bool bActive)
{
	if (bSpawningActive == bActive) return;

	bSpawningActive = bActive;
	OnSpawningActiveChanged.Broadcast(bSpawningActive);
}
