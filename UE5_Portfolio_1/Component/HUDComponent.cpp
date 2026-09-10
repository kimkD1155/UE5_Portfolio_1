// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "../Props/Barricade.h"
#include "../Component/InventoryComponent.h"
#include "../Weapon/RangedWeapon.h"
#include "../Core/KangGameState.h"
#include "../Manager/EnemyManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UHUDComponent::UHUDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(GetWorld(), CrosshairWidgetClass);
		if (CrosshairWidget)
			CrosshairWidget->AddToViewport();
	}

	if (InteractHintWidgetClass)
	{
		InteractHintWidget = CreateWidget<UInteractHintWidget>(GetWorld(), InteractHintWidgetClass);
		if (InteractHintWidget)
		{
			InteractHintWidget->AddToViewport();
			InteractHintWidget->HideHint();
		}
	}

	if (AmmoWidgetClass)
	{
		AmmoWidget = CreateWidget<UAmmoWidget>(GetWorld(), AmmoWidgetClass);
		if (AmmoWidget)
		{
			AmmoWidget->AddToViewport();
			AmmoWidget->ShowAmmoUI();
		}
	}

	if (BarricadeWidgetClass)
	{
		BarricadeWidget = CreateWidget<UBarricadeWidget>(GetWorld(), BarricadeWidgetClass);
		if (BarricadeWidget)
		{
			BarricadeWidget->AddToViewport();
		}
	}
	if (CoinWidgetClass)
	{
		CoinWidget = CreateWidget<UCoinWidget>(GetWorld(), CoinWidgetClass);
		if (CoinWidget)
			CoinWidget->AddToViewport();
	}

	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BarricadeWidgetClass is not set in HUDComponent."));
	}

	TArray<AActor*> Barricades;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABarricade::StaticClass(), Barricades);
	if (Barricades.Num() > 0)
	{
		ABarricade* Barricade = Cast<ABarricade>(Barricades[0]);
		if (BarricadeWidget && Barricade)
		{
			BarricadeWidget->InitWidget(Barricade);
			Barricade->OnHPChanged.AddDynamic(this, &UHUDComponent::UpdateBarricadeUI);
		}
	}

	// 탄약 UI: 인벤토리의 무기 교체 이벤트에 바인딩, 이미 무기가 있으면 즉시 반영
	if (OwnerCharacter)
	{
		if (UInventoryComponent* Inventory = OwnerCharacter->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->OnWeaponEquipped.AddDynamic(this, &UHUDComponent::HandleWeaponEquipped);
			if (AWeaponBase* Current = Inventory->GetEquippedWeapon())
			{
				HandleWeaponEquipped(Current);
			}
		}
	}

	// 국면(낮/밤) UI
	if (PhaseWidgetClass)
	{
		PhaseWidget = CreateWidget<UPhaseWidget>(GetWorld(), PhaseWidgetClass);
		if (PhaseWidget)
		{
			PhaseWidget->AddToViewport();
		}
	}

	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		GS->OnPhaseChanged.AddDynamic(this, &UHUDComponent::HandlePhaseChanged);
		GS->OnPhaseTimeChanged.AddDynamic(this, &UHUDComponent::HandlePhaseTimeChanged);
		HandlePhaseChanged(GS->GetCurrentPhase()); // 초기 상태 반영
	}

	if (UEnemyManager* EM = GetWorld()->GetSubsystem<UEnemyManager>())
	{
		EM->OnEnemyCountChanged.AddUObject(this, &UHUDComponent::HandleEnemyCountChanged);
	}
}

void UHUDComponent::HandlePhaseChanged(EGamePhase NewPhase)
{
	AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;
	const int32 DayNumber = GS ? GS->GetDayNumber() : 1;

	if (PhaseWidget)
	{
		PhaseWidget->OnPhaseUpdated(NewPhase, DayNumber);
	}

	if (NewPhase == EGamePhase::GameOver)
	{
		if (!ResultWidget && ResultWidgetClass)
		{
			ResultWidget = CreateWidget<UResultWidget>(GetWorld(), ResultWidgetClass);
			if (ResultWidget)
			{
				ResultWidget->AddToViewport(10);
				ResultWidget->OnResultShown(DayNumber);
			}
		}

		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetShowMouseCursor(true);
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void UHUDComponent::HandlePhaseTimeChanged(float Remaining)
{
	if (PhaseWidget)
	{
		PhaseWidget->OnTimeUpdated(Remaining);
	}
}

void UHUDComponent::HandleEnemyCountChanged(int32 NewCount)
{
	if (PhaseWidget)
	{
		PhaseWidget->OnEnemiesLeftUpdated(NewCount);
	}
}

void UHUDComponent::HandleWeaponEquipped(AWeaponBase* NewWeapon)
{
	if (BoundAmmoWeapon)
	{
		BoundAmmoWeapon->OnAmmoChanged.RemoveDynamic(this, &UHUDComponent::HandleAmmoChanged);
	}

	BoundAmmoWeapon = Cast<ARangedWeapon>(NewWeapon);

	if (BoundAmmoWeapon)
	{
		BoundAmmoWeapon->OnAmmoChanged.AddDynamic(this, &UHUDComponent::HandleAmmoChanged);
		HandleAmmoChanged(BoundAmmoWeapon->GetCurrentAmmo(), BoundAmmoWeapon->GetReserveAmmo());
	}
}

void UHUDComponent::HandleAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo)
{
	const FText Name = BoundAmmoWeapon ? BoundAmmoWeapon->GetWeaponName() : FText::GetEmpty();
	UpdateAmmoUI(CurrentAmmo, ReserveAmmo, Name);
}


// Called every frame
void UHUDComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

void UHUDComponent::ShowInteractHint(const FText& Text)
{
	if (!InteractHintWidget) return;
	InteractHintWidget->SetHintText(Text);
	InteractHintWidget->ShowHint();
}

void UHUDComponent::HideInteractHint()
{
	if (!InteractHintWidget) return;
	InteractHintWidget->HideHint();
}

void UHUDComponent::UpdateAmmoUI(int32 CurrentAmmo, int32 ReserveAmmo, const FText& WeaponName)
{
	if (!AmmoWidget) return;
	AmmoWidget->UpdateAmmo(CurrentAmmo, ReserveAmmo);
	AmmoWidget->UpdateWeaponName(WeaponName);
	AmmoWidget->ShowAmmoUI();
}

void UHUDComponent::InitBarricadeUI(ABarricade* Barricade)
{
	if (!BarricadeWidget || !Barricade) return;
	BarricadeWidget->InitWidget(Barricade);
	Barricade->OnHPChanged.AddDynamic(this, &UHUDComponent::UpdateBarricadeUI); // 바인딩
}

void UHUDComponent::UpdateBarricadeUI(float CurrentHP, float MaxHP)
{
	if (!BarricadeWidget) return;

	BarricadeWidget->UpdateHP(CurrentHP, MaxHP);
}

void UHUDComponent::UpdateCoinUI(int32 CurrentCoin)
{
	if (!CoinWidget) return;
	CoinWidget->UpdateCoin(CurrentCoin);
}
