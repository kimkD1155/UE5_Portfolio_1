// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

UINTERFACE(MinimalAPI)
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * UActorPoolSubsystem 이 재사용 직전/직후에 호출해주는 훅.
 * 풀 자체는 가시성/콜리전/Tick 같은 공통 상태만 만지고, 클래스별로 필요한
 * 특수 초기화(체력 리셋, 타이머 재무장 등)는 이 인터페이스를 구현해서 각자 처리한다.
 * (IWeaponHolder 와 같은 패턴 — 풀은 구현체의 구체 타입을 몰라도 된다.)
 */
class IPoolable
{
	GENERATED_BODY()

public:
	// 풀에서 꺼내져 다시 활동을 시작하기 직전 호출. "새로 스폰된 것처럼" 자기 상태를 초기화한다.
	virtual void OnAcquiredFromPool() {}

	// 다 쓰고 풀로 반납되기 직전 호출. 진행 중이던 타이머/이펙트 등을 정리한다.
	virtual void OnReturnedToPool() {}
};
