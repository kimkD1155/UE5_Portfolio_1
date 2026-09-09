// Fill out your copyright notice in the Description page of Project Settings.


#include "Barricade.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "../Component/HealthComponent.h"

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
	HealthComponent->SetMaxHealth(90.f);
}

// Called when the game starts or when spawned
void ABarricade::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &ABarricade::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &ABarricade::HandleDeath);
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
	OnBarricadeDestroyed();
}

void ABarricade::OnBarricadeDestroyed()
{
	BlockingVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 파괴 연출은 BP 에서 이벤트 바인딩해서 처리
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
