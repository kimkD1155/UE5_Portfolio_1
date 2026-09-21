// Fill out your copyright notice in the Description page of Project Settings.


#include "Barricade.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "../Manager/BarricadeManager.h"
#include "../AI/EnemyAIController.h"
#include "Engine/StaticMesh.h"

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
}

// Called when the game starts or when spawned
void ABarricade::BeginPlay()
{
	Super::BeginPlay();

	OnTakeAnyDamage.AddDynamic(this, &ABarricade::HandleTakeAnyDamage);

	if (UBarricadeManager* Manager = GetWorld()->GetSubsystem<UBarricadeManager>())
	{
		Manager->OnHPChanged.AddDynamic(this, &ABarricade::HandleSharedHPChanged);
		Manager->RegisterBarricade(this, BaseMaxHealth);
	}
}

void ABarricade::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (const UStaticMesh* StaticMeshAsset = Mesh->GetStaticMesh())
	{
		const FBoxSphereBounds LocalBounds = StaticMeshAsset->GetBounds();
		BlockingVolume->SetBoxExtent(LocalBounds.BoxExtent);
		BlockingVolume->SetRelativeLocation(LocalBounds.Origin);
	}
}

void ABarricade::HandleTakeAnyDamage(AActor* /*DamagedActor*/, float Damage, const UDamageType* /*DamageType*/,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f) return;

	// 좀비의 컨트롤러(AEnemyAIController)가 가한 피해만 받는다.
	if (!Cast<AEnemyAIController>(InstigatedBy)) return;

	if (UBarricadeManager* Manager = GetWorld()->GetSubsystem<UBarricadeManager>())
	{
		Manager->ApplyDamage(Damage, DamageCauser);
	}
}

// Called every frame
void ABarricade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABarricade::GetCurrentHealth() const
{
	const UBarricadeManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UBarricadeManager>() : nullptr;
	return Manager ? Manager->GetHealth() : 0.f;
}

float ABarricade::GetMaxHealth() const
{
	const UBarricadeManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UBarricadeManager>() : nullptr;
	return Manager ? Manager->GetMaxHealth() : 0.f;
}

bool ABarricade::IsDestroyed() const
{
	const UBarricadeManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UBarricadeManager>() : nullptr;
	return Manager && Manager->IsDestroyed();
}

bool ABarricade::IsFullHealth() const
{
	const UBarricadeManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UBarricadeManager>() : nullptr;
	return Manager && Manager->IsFullHealth();
}

void ABarricade::HandleSharedHPChanged(float Health, float MaxHealth)
{
	OnHPChanged.Broadcast(Health, MaxHealth);
}

void ABarricade::NotifyDestroyedByManager()
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
	UBarricadeManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UBarricadeManager>() : nullptr;
	if (!Manager || Manager->IsDestroyed() || Manager->IsFullHealth())
	{
		return;
	}

	Manager->Repair(RepairAmountPerInteract);
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
