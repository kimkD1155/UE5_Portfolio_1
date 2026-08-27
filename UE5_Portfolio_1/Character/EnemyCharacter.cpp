// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "../Core/EnemyAIController.h"
#include "../Core/KangPlayerState.h"
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
	CurrentHealth = MaxHealth;
	OnTakeAnyDamage.AddDynamic(this, &AEnemyCharacter::TakeDamageHandler);
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

void AEnemyCharacter::TakeDamageHandler(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (CurrentHealth <= 0.f) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("Enemy HP: %.1f / %.1f"), CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		Die();
		return;
	}

	PlayHitReactionMontage(HitReactionMontage);
}

void AEnemyCharacter::Die()
{
	// 플레이어 코인 지급
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
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
	bIsDead = true;
	PlayDieMontage(DieMontage);
}

AActor* AEnemyCharacter::GetTargetLocation() const
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
	if (!MontageToPlay) return;

	PlayAnimMontage(MontageToPlay);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AEnemyCharacter::OnDieMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
	}
}

void AEnemyCharacter::PlayAttackMontage()
{
	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy Attack Montage is null"));
		return;
	}
	

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(AttackMontage);

	FOnMontageEnded EndDelegate; // 이름 있는 변수(lvalue)로 먼저 생성
	EndDelegate.BindUObject(this, &AEnemyCharacter::OnAttackMontageEndedInternal);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
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
	UE_LOG(LogTemp, Warning, TEXT("Enemy Attack Montage Ended"));
	OnAttackMontageEnded.Broadcast();
}


void AEnemyCharacter::PlayHitReactionMontage(UAnimMontage* MontageToPlay)
{
	if (bIsPlayingHitReaction) return; // 재생 중이면 차단

	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy Hit Reaction Montage is null"));
		return;
	}
	bIsPlayingHitReaction = true; // 재생 시작

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->Montage_Play(HitReactionMontage);

	FOnMontageEnded EndDelegate; // 이름 있는 변수(lvalue)로 먼저 생성
	EndDelegate.BindUObject(this, &AEnemyCharacter::OnHitReactionMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, HitReactionMontage);


	GetCharacterMovement()->MaxWalkSpeed = 0.f; // 이동 속도를 0으로 설정하여 이동을 멈춤
}

void AEnemyCharacter::OnDieMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Die Montage Ended"));
	Destroy();
}

void AEnemyCharacter::OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Hit Reaction Montage Ended"));
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed; // 이동 속도를 원래대로 복원
	bIsPlayingHitReaction = false; // 재생 종료
}