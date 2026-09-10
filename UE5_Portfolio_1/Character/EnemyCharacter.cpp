// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "../Core/EnemyAIController.h"
#include "../Core/KangPlayerState.h"
#include "../Component/HealthComponent.h"
#include "../Animation/MontageHelper.h"
#include "../UE5_Portfolio_1.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "../Props/Barricade.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

namespace
{
	// ResolveHitZone 이 반환하는 존 태그. 오타 방지용 상수.
	const FName ZoneHead(TEXT("Head"));
	const FName ZoneLimb(TEXT("Limb"));
	const FName ZoneTorso(TEXT("Torso"));
}

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	// 부위별 배율을 직접 곱해 전달하므로 자동 바인딩을 끈다. (아래 OnTakePointDamage 로 대체)
	HealthComponent->SetBindToOwnerDamage(false);

	AttackHitBox = CreateDefaultSubobject<USphereComponent>(TEXT("AttackHitBox"));
	AttackHitBox->SetupAttachment(GetMesh(), FName("AttackHitBoxSocket"));
	AttackHitBox->SetSphereRadius(50.f);
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnAttackHitBoxOverlap);
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 히트스캔이 캡슐을 통과해 스켈레탈 메시(본 단위)에 맞도록 채널을 분리한다.
	// BP 에서 컴포넌트 콜리전을 덮어써도 항상 적용되도록 생성자가 아닌 BeginPlay 에서 설정.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Weapon, ECR_Ignore);
	if (GetMesh()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	GetMesh()->SetCollisionResponseToChannel(ECC_Weapon, ECR_Block);

	// 서브클래스 생성자에서 세팅한 MaxHealth 를 컴포넌트에 반영한 뒤 이벤트를 구독한다.
	HealthComponent->SetMaxHealth(MaxHealth);
	HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacter::HandleDeath);

	// 무기 히트스캔 · 투사체 모두 ApplyPointDamage 로 들어온다. 부위 판정을 위해 이 경로만 사용.
	OnTakePointDamage.AddDynamic(this, &AEnemyCharacter::HandlePointDamage);
}

void AEnemyCharacter::HandlePointDamage(AActor* /*DamagedActor*/, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* /*HitComp*/, FName BoneName,
	FVector /*ShotFromDirection*/, const UDamageType* /*DamageType*/, AActor* DamageCauser)
{
	if (Damage <= 0.f) return;

	AActor* DamageInstigator = DamageCauser;
	if (!DamageInstigator && InstigatedBy)
	{
		DamageInstigator = InstigatedBy->GetPawn();
	}

	const FName Zone = ResolveHitZone(BoneName);
	const float Multiplier = GetZoneMultiplier(Zone);
	const float FinalDamage = Damage * Multiplier;

	HealthComponent->ApplyDamage(FinalDamage, DamageInstigator);

	DrawHitZoneDebug(Zone, BoneName, HitLocation, Multiplier, FinalDamage);
}

FName AEnemyCharacter::ResolveHitZone(FName BoneName) const
{
	if (BoneName.IsNone()) return ZoneTorso; // 물리 애셋이 없거나 캡슐에 맞은 경우 → 몸통 취급

	const FString Bone = BoneName.ToString().ToLower();
	for (const FName& Keyword : HeadBoneKeywords)
	{
		if (Bone.Contains(Keyword.ToString())) return ZoneHead;
	}
	for (const FName& Keyword : LimbBoneKeywords)
	{
		if (Bone.Contains(Keyword.ToString())) return ZoneLimb;
	}
	return ZoneTorso;
}

float AEnemyCharacter::GetZoneMultiplier(FName Zone) const
{
	if (Zone == ZoneHead) return HeadDamageMultiplier;
	if (Zone == ZoneLimb) return LimbDamageMultiplier;
	return 1.f; // 몸통 기본
}

void AEnemyCharacter::DrawHitZoneDebug(FName Zone, FName BoneName, const FVector& HitLocation,
	float Multiplier, float FinalDamage) const
{
#if ENABLE_DRAW_DEBUG
	if (!bDrawHitZoneDebug) return;

	FColor Color = FColor::Yellow;          // 몸통
	if (Zone == ZoneHead) Color = FColor::Red;
	else if (Zone == ZoneLimb) Color = FColor::Cyan;

	DrawDebugSphere(GetWorld(), HitLocation, 8.f, 12, Color, false, HitZoneDebugDuration);
	DrawDebugString(GetWorld(), HitLocation + FVector(0.f, 0.f, 22.f),
		FString::Printf(TEXT("%s  x%.2f  %.0f dmg  [%s]"),
			*Zone.ToString(), Multiplier, FinalDamage,
			BoneName.IsNone() ? TEXT("no bone") : *BoneName.ToString()),
		nullptr, Color, HitZoneDebugDuration, true);
#endif
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float AEnemyCharacter::GetCurrentHealth() const
{
	return HealthComponent->GetHealth();
}

bool AEnemyCharacter::IsDead() const
{
	return HealthComponent->IsDead();
}

void AEnemyCharacter::HandleHealthChanged(float /*Health*/, float /*MaxHP*/, float Delta, AActor* /*DamageInstigator*/)
{
	// 피해를 입었고 아직 살아있으면 피격 리액션
	if (Delta < 0.f && !HealthComponent->IsDead())
	{
		PlayHitReactionMontage(HitReactionMontage);
	}
}

void AEnemyCharacter::HandleDeath(AActor* /*DamageInstigator*/)
{
	Die();
}

void AEnemyCharacter::Die()
{
	// 플레이어에게 코인 보상
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AKangPlayerState* PS = PC->GetPlayerState<AKangPlayerState>())
		{
			PS->AddCoin(CoinReward);
		}
	}

	if (AController* AC = GetController())
	{
		AC->UnPossess();
	}
	SetActorEnableCollision(false);
	PlayDieMontage(DieMontage);
}

AActor* AEnemyCharacter::GetTargetActor() const
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			return Cast<AActor>(BB->GetValueAsObject(TEXT("TargetLocation")));
		}
	}
	return nullptr;
}

void AEnemyCharacter::PlayDieMontage(UAnimMontage* MontageToPlay)
{
	MontageHelper::PlayWithEndCallback(GetMesh(), MontageToPlay, this, &AEnemyCharacter::OnDieMontageEnded);
}

void AEnemyCharacter::PlayAttackMontage()
{
	MontageHelper::PlayWithEndCallback(GetMesh(), AttackMontage, this, &AEnemyCharacter::OnAttackMontageEndedInternal);
}

void AEnemyCharacter::EnableAttackHitBox()
{
	HitActors.Empty();
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemyCharacter::DisableAttackHitBox()
{
	AttackHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyCharacter::OnAttackHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;
	if (HitActors.Contains(OtherActor)) return;

	ABarricade* Barricade = Cast<ABarricade>(OtherActor);
	if (!Barricade) return;

	HitActors.Add(OtherActor);

	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, nullptr);
}

void AEnemyCharacter::OnAttackMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted)
{
	OnAttackMontageEnded.Broadcast();
}


void AEnemyCharacter::PlayHitReactionMontage(UAnimMontage* MontageToPlay)
{
	if (bIsPlayingHitReaction || !MontageToPlay) return;

	bIsPlayingHitReaction = true;
	MontageHelper::PlayWithEndCallback(GetMesh(), MontageToPlay, this, &AEnemyCharacter::OnHitReactionMontageEnded);
	GetCharacterMovement()->MaxWalkSpeed = 0.f; // 피격 중 이동 정지
}

void AEnemyCharacter::OnDieMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Destroy();
}

void AEnemyCharacter::OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; // 이동 속도 복구
	bIsPlayingHitReaction = false;
}
