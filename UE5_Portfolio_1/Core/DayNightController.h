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

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	// 국면 변경 시 BP 에서 추가 연출(사운드, 파티클 등)을 붙일 훅
	UFUNCTION(BlueprintImplementableEvent, Category = "DayNight")
	void OnPhaseVisualChanged(EGamePhase NewPhase);

	void StartTransitionTo(const FLightPhaseSettings& Target);
	void ApplySettings(const FLightPhaseSettings& S, bool bRecaptureSky);

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
};
