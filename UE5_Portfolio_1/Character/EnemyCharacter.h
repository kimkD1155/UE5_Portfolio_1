// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;
class UAnimMontage;

DECLARE_DELEGATE_TwoParams(FOnMontageEnded, UAnimMontage*, bool /*bInterrupted*/);
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
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure)
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

protected:

	UFUNCTION()
	void TakeDamageHandler(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	virtual void Attack() {}

	void Die();

	AActor* GetTargetLocation() const;

	// 한 번 공격 시 데미지
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackDamage = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MaxHealth = 40.f;

	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 CoinReward = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MoveSpeed = 100.f;

	// 방어벽과의 Distance가 최소 188 정도 되므로, 자식 Enemy에서 그 이상으로 해야 됨
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackRange = 100.f;

	UPROPERTY()
	bool bIsPlayingHitReaction = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	USphereComponent* AttackHitBox;
	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
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

	// AnimNotify에서 호출
	void EnableAttackHitBox();
	void DisableAttackHitBox();

private:
	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION()
	void OnAttackHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnAttackMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted);

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
};
