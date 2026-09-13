// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "../Props/Barricade.h"
#include "../Component/InventoryComponent.h"
#include "../Weapon/RangedWeapon.h"
#include "../Core/KangGameState.h"
#include "../Core/KangPlayerGameModeBase.h"
#include "../Core/KangPlayerState.h"
#include "../Core/SaveGameSubsystem.h"
#include "Engine/GameInstance.h"
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

	// 커맨드 메뉴(B) — 구 AShop 위젯과 동일하게 숨긴 채로 미리 만들어둔다.
	if (MenuWidgetClass)
	{
		MenuWidget = CreateWidget<UShopWidget>(GetWorld(), MenuWidgetClass);
		if (MenuWidget)
		{
			MenuWidget->AddToViewport();
			MenuWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 일시정지 메뉴도 동일한 패턴
	if (PauseWidgetClass)
	{
		PauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseWidgetClass);
		if (PauseWidget)
		{
			PauseWidget->AddToViewport();
			PauseWidget->SetVisibility(ESlateVisibility::Hidden);
		}
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

	// 새 낮이 시작되면 수색 1회 제한을 초기화 (구 AScavengePoint::HandlePhaseChanged 로직)
	if (NewPhase == EGamePhase::Day && MenuWidget)
	{
		MenuWidget->ResetDailyScavenge();
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

bool UHUDComponent::IsMenuOpen() const
{
	return MenuWidget && MenuWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UHUDComponent::ToggleMenu()
{
	if (!MenuWidget) return;

	if (IsMenuOpen())
	{
		CloseMenu();
		return;
	}

	if (IsPaused()) return; // 일시정지 중엔 커맨드 메뉴를 열지 않는다 (동시에 두 메뉴가 뜨는 것 방지)

	MenuWidget->SetVisibility(ESlateVisibility::Visible);
	MenuWidget->RefreshCatalog();

	if (APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr)
	{
		PC->SetShowMouseCursor(true);

		// 위젯에 실제로 키보드 포커스를 줘야 위젯 자신의 On Key Down 이 확실히 불린다.
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

bool UHUDComponent::IsPaused() const
{
	return PauseWidget && PauseWidget->GetVisibility() == ESlateVisibility::Visible;
}

void UHUDComponent::TogglePause()
{
	if (!PauseWidget) return;

	if (IsPaused())
	{
		ResumeGame();
		return;
	}

	// B 메뉴가 열려있거나 이미 게임이 끝났으면 일시정지하지 않는다.
	if (IsMenuOpen()) return;
	const AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;
	if (GS && GS->IsGameOver()) return;

	PauseWidget->SetVisibility(ESlateVisibility::Visible);

	if (AKangPlayerGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>() : nullptr)
	{
		GM->SetGamePaused(true);
	}

	if (APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr)
	{
		PC->SetShowMouseCursor(true);

		// SetGamePaused 중에는 게임플레이 입력 바인딩(Enhanced Input 포함)이 기본적으로
		// 처리되지 않는다. ESC로 닫는 것까지 포함해서 Pause 위젯 자신의 키 입력으로
		// 처리해야 하므로, 반드시 이 위젯에 키보드 포커스를 줘야 한다.
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(PauseWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void UHUDComponent::ResumeGame()
{
	if (!IsPaused()) return;

	PauseWidget->SetVisibility(ESlateVisibility::Hidden);

	if (AKangPlayerGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>() : nullptr)
	{
		GM->SetGamePaused(false);
	}

	if (APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UHUDComponent::SaveGame()
{
	USaveGameSubsystem* Save = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<USaveGameSubsystem>() : nullptr;
	if (!Save) return;

	const AKangPlayerState* PS = OwnerCharacter ? OwnerCharacter->GetPlayerState<AKangPlayerState>() : nullptr;
	if (!PS) return;

	const AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;
	Save->SaveRun(PS->GetCoin(), PS->GetUpgradeLevelsMap(), GS ? GS->GetDayNumber() : 1);
}

void UHUDComponent::CloseMenu()
{
	if (!IsMenuOpen()) return;

	MenuWidget->SetVisibility(ESlateVisibility::Hidden);

	if (APlayerController* PC = OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}
