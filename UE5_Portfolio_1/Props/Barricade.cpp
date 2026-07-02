// Fill out your copyright notice in the Description page of Project Settings.


#include "Barricade.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

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
	OnTakeAnyDamage.AddDynamic(this, &ABarricade::TakeDamageHandler);

	CurrentHealth = MaxHealth;
}

// Called every frame
void ABarricade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABarricade::TakeDamageHandler(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	/*UE_LOG(LogTemp, Warning, TEXT("TakeDamageHandler called: %.1f"), Damage);*/

	if (IsDestroyed())
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	OnHPChanged.Broadcast(CurrentHealth, MaxHealth); 

	if (IsDestroyed())
	{
		OnBarricadeDestroyed();
	}
}

void ABarricade::Repair(float RepairAmount)
{
	if (IsDestroyed())
	{
		return; // 파괴 상태에서는 수리 불가 (재건축 정책이면 별도 처리)
	}

	CurrentHealth = FMath::Clamp(CurrentHealth + RepairAmount, 0.f, MaxHealth);
	OnHPChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ABarricade::OnBarricadeDestroyed()
{
	BlockingVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 파괴 연출은 BP에서 이벤트 바인딩해서 처리
}

// ── IInteractableInterface 구현 ─────────────────────────────────────

void ABarricade::Interact_Implementation(ACharacter* Interactor)
{
	if (IsDestroyed())
	{
		UE_LOG(LogTemp, Warning, TEXT("Barricade is destroyed, cannot repair"));
		return;
	}

	if (IsFullHealth())
	{
		UE_LOG(LogTemp, Warning, TEXT("Barricade already at full health"));
		return;
	}

	Repair(RepairAmountPerInteract);
	UE_LOG(LogTemp, Warning, TEXT("Barricade repaired by %s: %.1f / %.1f"), *Interactor->GetName(), CurrentHealth, MaxHealth);
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
	return FText::FromString(TEXT("Repiar E"));
}