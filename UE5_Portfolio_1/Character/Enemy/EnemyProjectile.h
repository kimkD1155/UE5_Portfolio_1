// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Interface/Poolable.h"
#include "EnemyProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class UE5_PORTFOLIO_1_API AEnemyProjectile : public AActor, public IPoolable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// IPoolable — 재사용/반납 시점의 상태 초기화 (UActorPoolSubsystem 이 호출)
	virtual void OnAcquiredFromPool() override;
	virtual void OnReturnedToPool() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	class UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float Damage = 10.f;

	// 발사 후(또는 Acquire 후) 아무것도 못 맞히면 이 시간 뒤 자동으로 풀에 반납된다.
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSpan = 5.f;

protected:
	// Destroy 대신 항상 이 함수로 정리한다 — 풀이 있으면 반납, 없으면(안전망) 파괴.
	void ReturnToPool();

private:
	FTimerHandle AutoReturnTimerHandle;
};
