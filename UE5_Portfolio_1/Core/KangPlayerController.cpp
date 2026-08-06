// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "../Ally/AllyBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "../Ally/AllyPlacementArea.h"
#include "Kismet/GameplayStatics.h"

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

	// 기존 고스트 프리뷰가 있으면 제거
	if (GhostPreview)
	{
		GhostPreview->Destroy();
		GhostPreview = nullptr;
	}

	PendingAllyClass = AllyClass;
	bIsInPlacementMode = true;

	// 고스트 프리뷰 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GhostPreview = GetWorld()->SpawnActor<AAllyBase>(AllyClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	if (GhostPreview)
	{
		if (UStaticMeshComponent* MeshComp = GhostPreview->FindComponentByClass<UStaticMeshComponent>())
		{
			if (GhostMaterial)
			{
				GhostMatInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
				for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
				{
					MeshComp->SetMaterial(i, GhostMatInstance);
				}
			}
		}
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

		bool bValid = IsValidPlacementLocation(HitResult.ImpactPoint);

		if (GhostMatInstance)
		{
			GhostMatInstance->SetVectorParameterValue(TEXT("Color"),
				bValid ? FLinearColor(0.f, 1.f, 0.f, 0.5f) : FLinearColor(1.f, 0.f, 0.f, 0.5f));
		}
	}
}

void AKangPlayerController::ConfirmPlacement()
{
	if (!bIsInPlacementMode || !GhostPreview) return;

	FVector PlacementLocation = GhostPreview->GetActorLocation();
	FRotator PlacementRotation = GhostPreview->GetActorRotation();

	if (!IsValidPlacementLocation(PlacementLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid placement location"));
		return;
	}

	

	// 고스트 먼저 제거
	GhostPreview->Destroy();
	GhostPreview = nullptr;


	// 실제 액터 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<AAllyBase>(PendingAllyClass, PlacementLocation, PlacementRotation, Params);

	bIsInPlacementMode = false;
	PendingAllyClass = nullptr;

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

}

void AKangPlayerController::CancelPlacement()
{
	if (GhostPreview)
	{
		GhostPreview->Destroy();
		GhostPreview = nullptr;
	}
	GhostMatInstance = nullptr;
	bIsInPlacementMode = false;
	PendingAllyClass = nullptr;

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

bool AKangPlayerController::IsValidPlacementLocation(const FVector& Location) const
{
	TArray<AActor*> Areas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAllyPlacementArea::StaticClass(), Areas);

	for (AActor* Actor : Areas)
	{
		if (AAllyPlacementArea* Area = Cast<AAllyPlacementArea>(Actor))
		{
			if (Area->IsLocationInside(Location))
			{
				return true;
			}
		}
	}
	return false;
}