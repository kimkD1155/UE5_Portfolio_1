// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../../Manager/ActorPoolSubsystem.h"

// Sets default values
AEnemyProjectile::AEnemyProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetSphereRadius(10.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComp->OnComponentHit.AddDynamic(this, &AEnemyProjectile::OnHit);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 800.f;
	ProjectileMovement->MaxSpeed = 800.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f; // 중력 없음

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 CollisionComp가 담당

	// 수명은 더 이상 SetLifeSpan(엔진의 자동 Destroy)에 맡기지 않는다 — 재사용될 때마다
	// OnAcquiredFromPool 에서 우리 타이머로 다시 무장해서, 만료되면 풀로 반납한다.
}

// Called when the game starts or when spawned
void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

}

void AEnemyProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyProjectile::OnAcquiredFromPool()
{
	ProjectileMovement->Velocity = FVector::ZeroVector;
	ProjectileMovement->Deactivate();

	GetWorldTimerManager().SetTimer(AutoReturnTimerHandle, this, &AEnemyProjectile::ReturnToPool, LifeSpan, false);
}

void AEnemyProjectile::OnReturnedToPool()
{
	GetWorldTimerManager().ClearTimer(AutoReturnTimerHandle);
	ProjectileMovement->Deactivate();
	ProjectileMovement->Velocity = FVector::ZeroVector;
}

void AEnemyProjectile::ReturnToPool()
{
	GetWorldTimerManager().ClearTimer(AutoReturnTimerHandle);

	if (UActorPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UActorPoolSubsystem>() : nullptr)
	{
		Pool->Release(this);
	}
	else
	{
		Destroy(); // 안전망 — 풀 서브시스템이 없는 상황(에디터 프리뷰 등)에서도 새지 않게
	}
}

void AEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		Damage,
		Hit.ImpactNormal,
		Hit,
		GetInstigator() ? GetInstigator()->GetController() : nullptr,
		this,
		nullptr
	);

	ReturnToPool();
}
