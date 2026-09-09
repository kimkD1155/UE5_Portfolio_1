// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;
class UAnimMontage;
class UHealthComponent;

DECLARE_MULTICAST_DELEGATE(FOnAttackMontageEnded);


UCLASS()
class UE5_PORTFOLIO_1_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure)
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

protected:

	// 체력/사망은 HealthComponent 가 담당한다. 이 캐릭터는 결과 이벤트만 구독한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UFUNCTION()
	void HandleHealthChanged(float Health, float MaxHealth, float Delta, AActor* DamageInstigator);

	UFUNCTION()
	void HandleDeath(AActor* DamageInstigator);

	virtual void Attack() {}

	void Die();

	// AActor::GetTargetLocation(AActor*) 와 이름이 겹치지 않도록 GetTargetActor 로 명명
	AActor* GetTargetActor() const;

	// 이 좀비가 한 대 때릴 때의 데미지
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackDamage = 30.f;

	// 서브클래스 생성자에서 HealthComponent 에 넣어줄 최대 체력
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MaxHealth = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 CoinReward = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MoveSpeed = 100.f;

	// 언리얼 기본 Distance 가 최소 188 정도 되므로, 자식 Enemy 에서 그 이상으로 해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackRange = 100.f;

	UPROPERTY()
	bool bIsPlayingHitReaction = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	USphereComponent* AttackHitBox;

	//────────────────────────────────────────────
	// 몽타주 및 애니메이션 관련
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DieMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	void PlayDieMontage(UAnimMontage* MontageToPlay);
	void PlayAttackMontage();
	void PlayHitReactionMontage(UAnimMontage* MontageToPlay);

	void OnDieMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnAttackMontageEnded OnAttackMontageEnded;

	// AnimNotify 에서 호출
	void EnableAttackHitBox();
	void DisableAttackHitBox();

private:
	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION()
	void OnAttackHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnAttackMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted);
};
