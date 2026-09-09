// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "../Core/EnemyAIController.h"
#include "../Core/KangPlayerState.h"
#include "../Component/HealthComponent.h"
#include "../Animation/MontageHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Animation/AnimMontage.h"
#include "../Props/Barricade.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

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

	// 서브클래스 생성자에서 세팅한 MaxHealth 를 컴포넌트에 반영한 뒤 이벤트를 구독한다.
	HealthComponent->SetMaxHealth(MaxHealth);
	HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacter::HandleHealthChanged);
	HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacter::HandleDeath);
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
