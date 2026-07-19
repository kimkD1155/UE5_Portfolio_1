// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "../Ally/AllyBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"

void AKangPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC, 0);
	}
}

void AKangPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInPlacementMode)
	{
		UpdateGhostPreview();
	}
}

void AKangPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ConfirmPlacement", IE_Pressed, this, &AKangPlayerController::ConfirmPlacement);
	InputComponent->BindAction("CancelPlacement", IE_Pressed, this, &AKangPlayerController::CancelPlacement);
}

void AKangPlayerController::StartPlacementMode(TSubclassOf<AAllyBase> AllyClass)
{
	if (!AllyClass) return;

	PendingAllyClass = AllyClass;
	bIsInPlacementMode = true;

	// 고스트 프리뷰 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GhostPreview = GetWorld()->SpawnActor<AAllyBase>(AllyClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (GhostPreview)
	{
		// 반투명 처리
		if (UStaticMeshComponent* MeshComp = GhostPreview->FindComponentByClass<UStaticMeshComponent>())
		{
			UMaterialInstanceDynamic* GhostMat = MeshComp->CreateAndSetMaterialInstanceDynamic(0);
			if (GhostMat)
			{
				GhostMat->SetScalarParameterValue(TEXT("Opacity"), 0.5f);
			}
		}
		// 콜리전 끄기
		GhostPreview->SetActorEnableCollision(false);
	}

	// 마우스 커서 표시
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());
}

void AKangPlayerController::UpdateGhostPreview()
{
	if (!GhostPreview) return;

	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		GhostPreview->SetActorLocation(HitResult.ImpactPoint);
	}
}

void AKangPlayerController::ConfirmPlacement()
{
	if (!bIsInPlacementMode || !GhostPreview) return;

	// 실제 액터 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<AAllyBase>(PendingAllyClass, GhostPreview->GetActorLocation(), FRotator::ZeroRotator, Params);

	CancelPlacement();
}

void AKangPlayerController::CancelPlacement()
{
	if (GhostPreview)
	{
		GhostPreview->Destroy();
		GhostPreview = nullptr;
	}

	bIsInPlacementMode = false;
	PendingAllyClass = nullptr;

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}