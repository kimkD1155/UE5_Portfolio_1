// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveGameSubsystem.h"
#include "KangSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FString SaveSlotName = TEXT("KangPortfolioSave");
	constexpr int32 SaveUserIndex = 0;
}

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadOrCreate();
}

void USaveGameSubsystem::LoadOrCreate()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		LoadedSave = Cast<UKangSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!LoadedSave)
	{
		LoadedSave = Cast<UKangSaveGame>(UGameplayStatics::CreateSaveGameObject(UKangSaveGame::StaticClass()));
	}
}

bool USaveGameSubsystem::HasSaveFile() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex);
}

void USaveGameSubsystem::SaveRun(int32 InCoin, const TMap<EUpgradeType, int32>& InUpgradeLevels, int32 InDayNumber)
{
	if (!LoadedSave) LoadOrCreate();

	LoadedSave->Coin = InCoin;
	LoadedSave->UpgradeLevels = InUpgradeLevels;
	LoadedSave->SavedDayNumber = InDayNumber;
	UGameplayStatics::SaveGameToSlot(LoadedSave, SaveSlotName, SaveUserIndex);
}

void USaveGameSubsystem::SaveBestDay(int32 DayReached)
{
	if (!LoadedSave) LoadOrCreate();
	if (DayReached <= LoadedSave->BestDayReached) return;

	LoadedSave->BestDayReached = DayReached;
	UGameplayStatics::SaveGameToSlot(LoadedSave, SaveSlotName, SaveUserIndex);
}

int32 USaveGameSubsystem::GetSavedCoin() const
{
	return LoadedSave ? LoadedSave->Coin : 100;
}

TMap<EUpgradeType, int32> USaveGameSubsystem::GetSavedUpgradeLevels() const
{
	return LoadedSave ? LoadedSave->UpgradeLevels : TMap<EUpgradeType, int32>();
}

int32 USaveGameSubsystem::GetSavedDayNumber() const
{
	return LoadedSave ? LoadedSave->SavedDayNumber : 1;
}

int32 USaveGameSubsystem::GetBestDayReached() const
{
	return LoadedSave ? LoadedSave->BestDayReached : 0;
}
