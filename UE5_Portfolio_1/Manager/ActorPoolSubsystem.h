// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolSubsystem.generated.h"

// 클래스 하나당 대기 중인(비활성) 인스턴스 목록. TMap 값 타입은 USTRUCT 여야 해서 감싼다.
USTRUCT()
struct FPooledActorList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

/**
 * 반복적으로 스폰/파괴되는 액터(투사체 등)를 Destroy 대신 재사용하는 범용 풀.
 * 특정 클래스를 모르는 채로 동작하고, 클래스별 특수 초기화는 IPoolable 를
 * 구현한 액터가 알아서 처리한다 (풀은 가시성/콜리전/Tick 같은 공통 상태만 다룬다).
 *
 * 지금은 AEnemyProjectile 에만 연결돼 있다. 좀비(캐릭터) 풀링은 AI 재빙의/전투 상태
 * 초기화가 훨씬 까다로워서 (재점유, 히트존 상태 등) 이번 범위에서는 의도적으로 제외했다 —
 * 필요해지면 AEnemyCharacter 가 같은 IPoolable 을 구현하는 식으로 확장하면 된다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API UActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 비활성 인스턴스가 있으면 재사용, 없으면 새로 스폰한다. 항상 유효한 액터를 반환한다
	// (SpawnActor 실패 시에만 nullptr).
	AActor* Acquire(TSubclassOf<AActor> Class, const FVector& Location, const FRotator& Rotation);

	template <typename T>
	T* Acquire(TSubclassOf<AActor> Class, const FVector& Location, const FRotator& Rotation)
	{
		return Cast<T>(Acquire(Class, Location, Rotation));
	}

	// 다 쓴 액터를 Destroy 대신 풀로 반납한다.
	void Release(AActor* Actor);

private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FPooledActorList> Pools;
};
