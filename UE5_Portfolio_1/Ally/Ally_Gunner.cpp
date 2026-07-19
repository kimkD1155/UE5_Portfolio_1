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

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GetActorLocation(),
        CurrentTarget->GetActorLocation(),
        ECC_Visibility,
        Params
    );

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