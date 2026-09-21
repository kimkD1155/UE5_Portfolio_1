// Fill out your copyright notice in the Description page of Project Settings.


#include "KangGameState.h"

void AKangGameState::SetPhase(EGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	OnPhaseChanged.Broadcast(CurrentPhase);
}

void AKangGameState::SetSpawningActive(bool bActive)
{
	if (bSpawningActive == bActive) return;

	bSpawningActive = bActive;
	OnSpawningActiveChanged.Broadcast(bSpawningActive);
}
