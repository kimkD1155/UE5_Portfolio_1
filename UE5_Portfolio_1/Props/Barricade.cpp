// Fill out your copyright notice in the Description page of Project Settings.


#include "Barricade.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../Component/HealthComponent.h"
#include "../Core/KangPlayerState.h"
#include "../Core/UpgradeType.h"

// Sets default values
ABarricade::ABarricade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	BlockingVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingVolume"));
	BlockingVolume->SetupAttachment(Mesh);
	BlockingVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BlockingVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetMaxHealth(BaseMaxHealth);
}

// Called when the game starts or when spawned
void ABarricade::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &ABarricade::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &ABarricade::HandleDeath);

	BindToPlayerState();
}

void ABarricade::BindToPlayerState()
{
	if (BoundPlayerState.IsValid()) return;

	AKangPlayerState* PS = nullptr;
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PS = PC->GetPlayerState<AKangPlayerState>();
	}

	if (!PS)
	{
		// PlayerState 가 아직 없으면 우선 기본 최대 체력으로 맞춰두고 잠시 후 재시도
		HandleUpgradesChanged();
		GetWorldTimerManager().SetTimer(BindRetryHandle, this, &ABarricade::BindToPlayerState, 0.25f, false);
		return;
	}

	BoundPlayerState = PS;
	PS->OnUpgradesChanged.AddDynamic(this, &ABarricade::HandleUpgradesChanged);
	HandleUpgradesChanged(); // 현재 업그레이드 상태로 최대 체력 초기화
}

void ABarricade::HandleUpgradesChanged()
{
	AKangPlayerState* PS = BoundPlayerState.Get();
	const float Multiplier = PS ? PS->GetUpgradeMultiplier(EUpgradeType::BarricadeHealth) : 1.f;

	const float OldMax = HealthComponent->GetMaxHealth();
	const float NewMax = BaseMaxHealth * Multiplier;

	// 최대치만 올리고 현재 체력은 유지 → 증가분만큼 회복시켜 준다
	HealthComponent->SetMaxHealth(NewMax, /*bFillToMax=*/false);
	if (NewMax > OldMax && !IsDestroyed())
	{
		HealthComponent->Heal(NewMax - OldMax);
	}
	else
	{
		// 최대치가 그대로여도 HP 바가 새 비율을 반영하도록 한 번 브로드캐스트
		OnHPChanged.Broadcast(HealthComponent->GetHealth(), NewMax);
	}
}

// Called every frame
void ABarricade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABarricade::GetCurrentHealth() const
{
	return HealthComponent->GetHealth();
}

float ABarricade::GetMaxHealth() const
{
	return HealthComponent->GetMaxHealth();
}

bool ABarricade::IsDestroyed() const
{
	return HealthComponent->IsDead();
}

bool ABarricade::IsFullHealth() const
{
	return HealthComponent->IsFullHealth();
}

void ABarricade::HandleHealthChanged(float Health, float MaxHealth, float /*Delta*/, AActor* /*DamageInstigator*/)
{
	OnHPChanged.Broadcast(Health, MaxHealth);
}

void ABarricade::HandleDeath(AActor* /*DamageInstigator*/)
{
	ApplyDestroyedState();
	OnBarricadeDestroyed.Broadcast(this);
}

void ABarricade::ApplyDestroyedState()
{
	BlockingVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 파괴 연출은 BP 에서 OnBarricadeDestroyed 바인딩해서 처리
}

// ── IInteractableInterface 구현 ───────────────────────────────────────────────

void ABarricade::Interact_Implementation(ACharacter* Interactor)
{
	if (IsDestroyed() || IsFullHealth())
	{
		return;
	}

	HealthComponent->Heal(RepairAmountPerInteract);
}

FText ABarricade::GetInteractHintText_Implementation()
{
	if (IsDestroyed())
	{
		return FText::FromString(TEXT("Destroyed"));
	}
	if (IsFullHealth())
	{
		return FText::FromString(TEXT("Barricade Max Health"));
	}
	return FText::FromString(TEXT("Repair E"));
}
