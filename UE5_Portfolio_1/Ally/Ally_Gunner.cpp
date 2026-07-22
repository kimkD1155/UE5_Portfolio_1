// Fill out your copyright notice in the Description page of Project Settings.


#include "Ally_Gunner.h"
#include "Kismet/GameplayStatics.h"

void AAlly_Gunner::Attack()
{
    CurrentTarget = FindTarget();

    if (!IsValid(CurrentTarget)) return;
    if (!HasLineOfSight(CurrentTarget)) return;
    if (!IsValid(CurrentTarget)) return;

    CurrentTarget = FindTarget();
    if (!CurrentTarget || !HasLineOfSight(CurrentTarget)) return;

    FVector StartLocation = GetActorLocation() + FVector(0, 0, TraceHeightOffset);
    FVector EndLocation = CurrentTarget->GetActorLocation() + FVector(0, 0, TraceHeightOffset);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        Params
    );

#if WITH_EDITOR
    // 타겟으로 향하는 라인 (초록: 공격 성공, 빨강: 실패)
    DrawDebugLine(
        GetWorld(),
        StartLocation,
        EndLocation,
        bHit ? FColor::Green : FColor::Red,
        false, 0.1f, 0, 2.f
    );

    // 타겟 위에 구체 표시
    if (IsValid(CurrentTarget))
    {
        DrawDebugSphere(
            GetWorld(),
            CurrentTarget->GetActorLocation(),
            50.f,
            12,
            FColor::Yellow,
            false, 0.1f
        );
    }
#endif

    if (bHit && IsValid(HitResult.GetActor()))
    {
        UGameplayStatics::ApplyPointDamage(
            HitResult.GetActor(),
            AttackDamage,
            (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal(),
            HitResult,
            nullptr,
            this,
            nullptr
        );
    }
}