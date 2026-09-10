// Fill out your copyright notice in the Description page of Project Settings.


#include "ScavengePoint.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "../Core/KangGameState.h"
#include "../Core/KangPlayerState.h"

AScavengePoint::AScavengePoint()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	// 상호작용 트레이스(UInteractionComponent)는 ECC_Visibility 를 쓴다 — 기본 StaticMesh 가 Block.
}

void AScavengePoint::BeginPlay()
{
	Super::BeginPlay();

	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		GS->OnPhaseChanged.AddDynamic(this, &AScavengePoint::HandlePhaseChanged);
		HandlePhaseChanged(GS->GetCurrentPhase());
	}
}

void AScavengePoint::HandlePhaseChanged(EGamePhase NewPhase)
{
	if (NewPhase == EGamePhase::Day)
	{
		bUsedThisDay = false;
		SetAvailable(true);
	}
	else
	{
		// 밤/게임오버에는 사용 불가 상태로 표시 (상호작용은 어차피 Interact 에서 막힘)
		SetAvailable(false);
	}
}

void AScavengePoint::SetAvailable(bool bAvailable)
{
	SetActorHiddenInGame(!bAvailable);
	SetActorEnableCollision(bAvailable);
}

bool AScavengePoint::CanScavengeNow() const
{
	const AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>();
	if (GS && GS->GetCurrentPhase() != EGamePhase::Day) return false;
	if (bOncePerDay && bUsedThisDay) return false;
	return true;
}

void AScavengePoint::Interact_Implementation(ACharacter* Interactor)
{
	if (!CanScavengeNow()) return;

	APlayerController* PC = Interactor ? Cast<APlayerController>(Interactor->GetController()) : nullptr;
	AKangPlayerState* PS = PC ? PC->GetPlayerState<AKangPlayerState>() : nullptr;
	if (!PS) return;

	PS->AddCoin(CoinReward);
	bUsedThisDay = true;

	if (bOncePerDay)
	{
		SetAvailable(false);
	}
}

FText AScavengePoint::GetInteractHintText_Implementation()
{
	const AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>();
	if (GS && GS->GetCurrentPhase() != EGamePhase::Day)
	{
		return FText::FromString(TEXT("밤에는 수색할 수 없다"));
	}
	if (bOncePerDay && bUsedThisDay)
	{
		return FText::FromString(TEXT("이미 수색함"));
	}
	return FText::FromString(FString::Printf(TEXT("수색  [E]   +%d"), CoinReward));
}
