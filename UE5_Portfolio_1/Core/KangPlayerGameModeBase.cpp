// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerGameModeBase.h"
#include "UObject/ConstructorHelpers.h"

AKangPlayerGameModeBase::AKangPlayerGameModeBase()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Character/Blueprint/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}
