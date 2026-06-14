// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GamePlayStatics.h"
#include "GameFramework/Character.h"

AMeleeWeapon::AMeleeWeapon()
{
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(RootComponent);

    // 평소엔 꺼둠 — 공격 모션 중에만 켜짐
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::OnHitBoxOverlap);
}

// ── 공격 시작 / 종료 ──────────────────────────────────────────────────────────
void AMeleeWeapon::StartFire()
{
    UE_LOG(LogTemp, Warning, TEXT("MeleeWeapon: StartFire"));
}

void AMeleeWeapon::StopFire()
{
    UE_LOG(LogTemp, Warning, TEXT("MeleeWeapon: StopFire"));
}

void AMeleeWeapon::StartAim()
{
    UE_LOG(LogTemp, Warning, TEXT("MeleeWeapon: StartAim"));
}

void AMeleeWeapon::StopAim()
{
    UE_LOG(LogTemp, Warning, TEXT("MeleeWeapon: StopAim"));
}

// ── 히트박스 제어 ─────────────────────────────────────────────────────────────
void AMeleeWeapon::EnableHitBox()
{
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMeleeWeapon::DisableHitBox()
{
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// ── 히트 판정 ─────────────────────────────────────────────────────────────────
void AMeleeWeapon::OnHitBoxOverlap(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult& SweepResult)
{
    // 자기 자신 / 소유자 / 이미 맞은 액터 제외
    if (!OtherActor) return;
    if (OtherActor == this || OtherActor == OwnerCharacter) return;
    if (HitActors.Contains(OtherActor)) return;

    HitActors.Add(OtherActor);

    UGameplayStatics::ApplyPointDamage(
        OtherActor,
        Damage,
        SweepResult.ImpactNormal,
        SweepResult,
        OwnerCharacter ? OwnerCharacter->GetController() : nullptr,
        this,
        nullptr
    );
}