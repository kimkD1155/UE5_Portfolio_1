// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;

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

protected:

	UFUNCTION()
	void TakeDamageHandler(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	virtual void Attack() {}

	void Die();

	AActor* GetTargetLocation() const;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MaxHealth = 40.f;

	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 CoinReward = 10;

	// 방어벽과의 Distance가 최소 188 정도 되므로, 자식 Enemy에서 그 이상으로 해야 됨
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackRange = 200.f;

};
