// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GamePhase.h"
#include "DayNightController.generated.h"

class ADirectionalLight;
class ASkyLight;

// 한 국면(낮 또는 밤)의 조명 상태. BP 인스턴스에서 값만 튜닝한다.
USTRUCT(BlueprintType)
struct FLightPhaseSettings
{
	GENERATED_BODY()

	// 태양(DirectionalLight) 액터 회전. Pitch 음수 = 하늘 위, 양수 = 지평선 아래.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
	FRotator SunRotation = FRotator(-45.f, -30.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0"))
	float SunIntensity = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
	FLinearColor SunColor = FLinearColor(1.f, 0.95f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0"))
	float SkyLightIntensity = 1.f;

	// 하늘에 보이는 태양 원반(SkyAtmosphere의 Sun Disk)을 표시할지. 태양 위치를 낮/밤 내내
	// 고정해두는 경우, 밤에도 하늘에 해가 떠있는 게 어색해서 밤에는 꺼두는 용도.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
	bool bShowSunDisc = true;
};

/**
 * 낮/밤 조명 전환을 담당한다. 로직(국면 구독, 조명 세팅, 보간)은 C++,
 * 튜닝값(FLightPhaseSettings)과 추가 연출은 BP.
 *
 * 레벨에 1개 배치. 태양/스카이라이트는 지정하지 않으면 BeginPlay 에서 자동 탐색한다.
 */
UCLASS()
class UE5_PORTFOLIO_1_API ADayNightController : public AActor
{
	GENERATED_BODY()

public:
	ADayNightController();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	// 국면 변경 시 BP 에서 추가 연출(사운드, 파티클 등)을 붙일 훅
	UFUNCTION(BlueprintImplementableEvent, Category = "DayNight")
	void OnPhaseVisualChanged(EGamePhase NewPhase);

	void StartTransitionTo(const FLightPhaseSettings& Target);
	void ApplySettings(const FLightPhaseSettings& S, bool bRecaptureSky);

	// BeginPlay 시점에 AKangGameState 가 아직 준비되지 않았을 수 있어(액터 초기화 순서 문제),
	// 성공할 때까지 하트비트 타이머에서 계속 재시도한다. 한 번 성공하면 더 이상 호출되지 않는다.
	void TryBindGameState();

	// Actor Tick 대신 타이머로 보간을 돌린다 — 배치된 인스턴스가 (BP 클래스 디폴트 등에서)
	// Tick 자체가 꺼져있어도 항상 동작하도록 보장하기 위해서다. 실제로 이 프로젝트에서
	// Tick 이 꺼져 있어서 전환이 통째로 멈춰있던 버그를 겪었다.
	void Heartbeat();
	FTimerHandle HeartbeatTimerHandle;
	static constexpr float HeartbeatInterval = 0.05f;

	static FLightPhaseSettings BlendSettings(const FLightPhaseSettings& A, const FLightPhaseSettings& B, float Alpha);

	UPROPERTY(EditAnywhere, Category = "DayNight")
	FLightPhaseSettings DaySettings;

	UPROPERTY(EditAnywhere, Category = "DayNight")
	FLightPhaseSettings NightSettings;

	// 전환에 걸리는 시간(초). 0 이면 즉시 스냅.
	UPROPERTY(EditAnywhere, Category = "DayNight", meta = (ClampMin = "0.0"))
	float TransitionTime = 2.5f;

	// 비워두면 BeginPlay 에서 GetActorOfClass 로 첫 번째 액터를 찾는다.
	UPROPERTY(EditInstanceOnly, Category = "DayNight")
	TObjectPtr<ADirectionalLight> SunLight;

	UPROPERTY(EditInstanceOnly, Category = "DayNight")
	TObjectPtr<ASkyLight> SkyLight;

private:
	FLightPhaseSettings FromSettings;
	FLightPhaseSettings ToSettings;
	float TransitionElapsed = 0.f;
	bool bTransitioning = false;
	bool bBoundToGameState = false;
};
