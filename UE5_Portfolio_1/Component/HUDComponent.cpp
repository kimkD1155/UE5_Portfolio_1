// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "../Manager/BarricadeManager.h"
#include "../Component/InventoryComponent.h"
#include "../Component/CombatComponent.h"
#include "../Weapon/RangedWeapon.h"
#include "../Core/KangGameState.h"
#include "../Core/KangPlayerGameModeBase.h"
#include "../Core/KangPlayerState.h"
#include "../Core/SaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "../Manager/EnemyManager.h"

// Sets default values for this component's properties
UHUDComponent::UHUDComponent()
{
	// 장전 진행률(원형 프로그레스바)을 매 프레임 크로스헤어 위젯에 밀어넣어야 해서 켜둔다.
	PrimaryComponentTick.bCanEverTick = true;

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

	// 여러 바리케이드가 하나의 공유 체력 풀을 쓰므로, 특정 인스턴스가 아니라
	// UBarricadeManager 를 직접 구독한다.
	if (BarricadeWidget)
	{
		if (UBarricadeManager* BM = GetWorld()->GetSubsystem<UBarricadeManager>())
		{
			BarricadeWidget->UpdateHP(BM->GetHealth(), BM->GetMaxHealth());
			BM->OnHPChanged.AddDynamic(this, &UHUDComponent::UpdateBarricadeUI);
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

		CombatComp = OwnerCharacter->FindComponentByClass<UCombatComponent>();
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

	// Day 안내판 — 아래의 초기 HandlePhaseChanged() 호출(및 GameMode가 이미 먼저 국면을
	// 전환해뒀을 경우의 첫 브로드캐스트)이 이 위젯을 바로 써야 하므로, 국면 구독보다 먼저
	// 만들어둔다. 순서가 바뀌면 게임 시작 직후의 첫 Night 전환(=Day 1 Start)을 놓친다.
	if (DayPhaseAnnounceWidgetClass)
	{
		DayPhaseAnnounceWidget = CreateWidget<UDayPhaseAnnounceWidget>(GetWorld(), DayPhaseAnnounceWidgetClass);
		if (DayPhaseAnnounceWidget)
		{
			DayPhaseAnnounceWidget->AddToViewport(50);
			DayPhaseAnnounceWidget->OnQueueFinished.AddUObject(this, &UHUDComponent::HandleDayAnnounceFinished);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DayPhaseAnnounceWidgetClass is not set in HUDComponent."));
	}

	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		GS->OnPhaseChanged.AddDynamic(this, &UHUDComponent::HandlePhaseChanged);
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

	// Day 시작/종료를 검은 화면 안내로 크게 알려준다. "시작"은 밤이 시작되는 순간, "종료"는
	// 그 밤이 끝나 낮이 되는 순간 — 서로 다른 두 전환 시점에 각각 뜬다. GS 의 DayNumber 는
	// 낮이 시작될 때 이미 다음 사이클 번호로 올라가 있으므로, "종료" 안내는 밤이 시작될 때
	// 캐싱해둔 번호를 그대로 써서 같은 사이클끼리 번호가 맞물리게 한다.
	if (DayPhaseAnnounceWidget && NewPhase == EGamePhase::Night)
	{
		CurrentCycleDayNumber = DayNumber;
		DayPhaseAnnounceWidget->ShowAnnouncement(
			FText::FromString(FString::Printf(TEXT("Day %d Start"), CurrentCycleDayNumber)));
		// 안내가 다 끝날 때까지(HandleDayAnnounceFinished) 웨이브/입력이 먼저 진행되지 않게 막는다.
		PauseForDayAnnouncement();
	}
	else if (DayPhaseAnnounceWidget && NewPhase == EGamePhase::Day && LastPhase == EGamePhase::Night)
	{
		DayPhaseAnnounceWidget->ShowAnnouncement(
			FText::FromString(FString::Printf(TEXT("Day %d End"), CurrentCycleDayNumber)));
		// 낮이 시작될 때도 상점이 열리는 게 안내판 뒤로 가려지는 동안엔 입력이 먼저 들어가면 안 된다.
		PauseForDayAnnouncement();
	}
	LastPhase = NewPhase;

	// 낮이 시작되면 상점을 자동으로 열고, 밤/게임오버가 되면 닫아 구매를 막는다.
	if (NewPhase == EGamePhase::Day)
	{
		OpenMenu();
	}
	else if (NewPhase == EGamePhase::Night || NewPhase == EGamePhase::GameOver)
	{
		CloseMenu();
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

void UHUDComponent::HandleEnemyCountChanged(int32 NewCount)
{
	if (PhaseWidget)
	{
		PhaseWidget->OnEnemiesLeftUpdated(NewCount);
	}
}

void UHUDComponent::PauseForDayAnnouncement()
{
	// SetGamePaused()는 절대 쓰지 않는다 — 실측 결과 UGameplayStatics::SetGamePaused 가
	// 안내판 위젯 자신의 FTimerManager 타이머까지 멈춰버려서, 화면이 "Day 1 Start"에서 영원히
	// 멈춰버리는 문제가 있었다 (안내판 스스로 끝날 방법이 없어짐). 대신 스포너만 개별적으로
	// 멈추고, 입력은 DisableInput으로 직접 막는다.
	if (AKangPlayerGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>() : nullptr)
	{
		GM->SetWaveSpawningPaused(true);
	}

	if (OwnerCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			OwnerCharacter->DisableInput(PC);
		}
	}
}

void UHUDComponent::HandleDayAnnounceFinished()
{
	if (AKangPlayerGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AKangPlayerGameModeBase>() : nullptr)
	{
		GM->SetWaveSpawningPaused(false);
	}

	if (OwnerCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			OwnerCharacter->EnableInput(PC);
		}
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

	if (CrosshairWidget && CombatComp)
	{
		CrosshairWidget->OnReloadProgressUpdated(CombatComp->IsReloading(), CombatComp->GetReloadProgress());
	}
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

	if (IsPaused()) return; // 일시정지 중엔 상점 메뉴를 열지 않는다 (동시에 두 메뉴가 뜨는 것 방지)

	// 밤에는 상점을 열 수 없다 — 구매는 낮에만 가능하다.
	const AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;
	if (GS && GS->GetCurrentPhase() != EGamePhase::Day) return;

	OpenMenu();
}

void UHUDComponent::OpenMenu()
{
	if (!MenuWidget || IsMenuOpen()) return;

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
	Save->SaveRun(PS->GetCoin(), PS->GetUpgradeLevelsMap(), GS ? GS->GetDayNumber() : 1, PS->GetUnlockedWeapons());
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
