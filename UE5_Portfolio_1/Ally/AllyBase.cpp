#include "AllyBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Character/EnemyCharacter.h"

AAllyBase::AAllyBase()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
    AttackRangeSphere->SetupAttachment(RootComponent);
    AttackRangeSphere->SetSphereRadius(AttackRange);
    AttackRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AttackRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AttackRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AAllyBase::OnEnemyEnterRange);
    AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AAllyBase::OnEnemyExitRange);
}

void AAllyBase::BeginPlay()
{
    Super::BeginPlay();
}

void AAllyBase::SetTargetPriority(ETargetPriority NewPriority)
{
    TargetPriority = NewPriority;
    CurrentTarget = FindTarget();
}

AActor* AAllyBase::FindTarget()
{
    EnemiesInRange.RemoveAll([](AActor* A) { return !IsValid(A); });
    if (EnemiesInRange.Num() == 0) return nullptr;

    switch (TargetPriority)
    {
    case ETargetPriority::Closest:       return FindClosestTarget();
    case ETargetPriority::LowestHP:      return FindLowestHPTarget();
    case ETargetPriority::ClosestToWall: return FindClosestToWallTarget();
    default:                             return FindClosestTarget();
    }
}

AActor* AAllyBase::FindClosestTarget()
{
    AActor* Closest = nullptr;
    float MinDist = TNumericLimits<float>::Max();

    for (AActor* Enemy : EnemiesInRange)
    {
        float Dist = FVector::DistSquared(GetActorLocation(), Enemy->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Closest = Enemy;
        }
    }
    return Closest;
}

AActor* AAllyBase::FindLowestHPTarget()
{
    AActor* LowestHP = nullptr;
    float MinHP = TNumericLimits<float>::Max();

    for (AActor* Enemy : EnemiesInRange)
    {
        if (AEnemyCharacter* EC = Cast<AEnemyCharacter>(Enemy))
        {
            if (EC->GetCurrentHealth() < MinHP)
            {
                MinHP = EC->GetCurrentHealth();
                LowestHP = EC;
            }
        }
    }
    return LowestHP;
}

AActor* AAllyBase::FindClosestToWallTarget()
{
    TArray<AActor*> Walls;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("DefenseWall"), Walls);
    if (Walls.Num() == 0) return FindClosestTarget();

    AActor* ClosestToWall = nullptr;
    float MinDist = TNumericLimits<float>::Max();

    for (AActor* Enemy : EnemiesInRange)
    {
        for (AActor* Wall : Walls)
        {
            float Dist = FVector::DistSquared(Enemy->GetActorLocation(), Wall->GetActorLocation());
            if (Dist < MinDist)
            {
                MinDist = Dist;
                ClosestToWall = Enemy;
            }
        }
    }
    return ClosestToWall;
}

bool AAllyBase::HasLineOfSight(AActor* Target)
{
    if (!IsValid(Target)) return false;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // 바리케이드 무시
    TArray<AActor*> Walls;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("DefenseWall"), Walls);
    for (AActor* Wall : Walls)
    {
        Params.AddIgnoredActor(Wall);
    }

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetActorLocation(),
        Target->GetActorLocation(),
        ECC_Pawn,
        Params
    );

    return !bHit || HitResult.GetActor() == Target;
}

void AAllyBase::OnEnemyEnterRange(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (Cast<AEnemyCharacter>(OtherActor))
    {
        EnemiesInRange.Add(OtherActor);
        if (!GetWorldTimerManager().IsTimerActive(AttackTimerHandle))
        {
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AAllyBase::AttackWrapper, AttackInterval, true);

        }
    }
}

void AAllyBase::OnEnemyExitRange(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    EnemiesInRange.Remove(OtherActor);
    if (EnemiesInRange.Num() == 0)
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        CurrentTarget = nullptr;
    }
}

void AAllyBase::AttackWrapper()
{
    Attack();
}