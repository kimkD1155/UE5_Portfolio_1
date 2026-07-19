// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AllyBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
	Closest,
	LowestHP,
	ClosestToWall
};

UCLASS(Abstract)
class UE5_PORTFOLIO_1_API AAllyBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAllyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

    UFUNCTION(BlueprintCallable)
    void SetTargetPriority(ETargetPriority NewPriority);

    UFUNCTION()
    void OnEnemyEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEnemyExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void AttackWrapper();
protected:

    AActor* FindTarget();
    AActor* FindClosestTarget();
    AActor* FindLowestHPTarget();
    AActor* FindClosestToWallTarget();
    bool HasLineOfSight(AActor* Target);

    virtual void Attack() PURE_VIRTUAL(AAllyBase::Attack, );

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* AttackRangeSphere;

    UPROPERTY(EditDefaultsOnly, Category = "Ally")
    float AttackRange = 1500.f;

    UPROPERTY(EditDefaultsOnly, Category = "Ally")
    float AttackDamage = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Ally")
    float AttackInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ally")
    ETargetPriority TargetPriority = ETargetPriority::Closest;

    UPROPERTY()
    TArray<AActor*> EnemiesInRange;

    UPROPERTY()
    AActor* CurrentTarget;

    FTimerHandle AttackTimerHandle;

};
