// Fill out your copyright notice in the Description page of Project Settings.


#include "DayNightController.h"
#include "KangGameState.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"

ADayNightController::ADayNightController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // 전환 중에만 틱

	// 낮 기본값은 struct 기본값을 그대로 쓰고, 밤 기본값만 밤답게 덮어쓴다.
	// 주의: Pitch 는 "빛이 향하는 방향" 이라 음수여야 아래로 비춘다 (양수를 쓰면 달빛이 위로
	// 새어나가 지면·캐릭터에 직접광이 전혀 안 닿아 완전히 캄캄해진다 — 실제로 겪었던 버그).
	// 게임플레이용 "밝은 밤"이라 사실적인 달빛 조도가 아니라 시인성 위주로 값을 올렸다.
	NightSettings.SunRotation = FRotator(-30.f, 140.f, 0.f);  // 달처럼 하늘 위에서 비스듬히 비춤
	NightSettings.SunIntensity = 1.5f;
	NightSettings.SunColor = FLinearColor(0.45f, 0.55f, 0.9f); // 차가운 달빛 톤이지만 밝게
	NightSettings.SkyLightIntensity = 0.6f;                    // 그림자 쪽도 완전히 안 죽게
}

void ADayNightController::BeginPlay()
{
	Super::BeginPlay();

	if (!SunLight)
	{
		SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(this, ADirectionalLight::StaticClass()));
	}
	if (!SkyLight)
	{
		SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(this, ASkyLight::StaticClass()));
	}

	if (AKangGameState* GS = GetWorld()->GetGameState<AKangGameState>())
	{
		GS->OnPhaseChanged.AddDynamic(this, &ADayNightController::HandlePhaseChanged);

		// 시작 국면을 즉시(스냅) 반영
		const EGamePhase Phase = GS->GetCurrentPhase();
		const FLightPhaseSettings& Initial = (Phase == EGamePhase::Night) ? NightSettings : DaySettings;
		FromSettings = ToSettings = Initial;
		ApplySettings(Initial, /*bRecaptureSky=*/true);
	}
}

void ADayNightController::HandlePhaseChanged(EGamePhase NewPhase)
{
	OnPhaseVisualChanged(NewPhase);

	switch (NewPhase)
	{
	case EGamePhase::Day:
		StartTransitionTo(DaySettings);
		break;
	case EGamePhase::Night:
		StartTransitionTo(NightSettings);
		break;
	default:
		// None / GameOver — 조명 변경 없음
		break;
	}
}

void ADayNightController::StartTransitionTo(const FLightPhaseSettings& Target)
{
	ToSettings = Target;

	if (TransitionTime <= KINDA_SMALL_NUMBER)
	{
		FromSettings = Target;
		ApplySettings(Target, true);
		bTransitioning = false;
		SetActorTickEnabled(false);
		return;
	}

	// 현재 적용된 값(FromSettings)에서 Target 으로 보간 시작
	TransitionElapsed = 0.f;
	bTransitioning = true;
	SetActorTickEnabled(true);
}

void ADayNightController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTransitioning) return;

	TransitionElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(TransitionElapsed / FMath::Max(TransitionTime, KINDA_SMALL_NUMBER), 0.f, 1.f);

	const FLightPhaseSettings Blended = BlendSettings(FromSettings, ToSettings, Alpha);

	if (Alpha >= 1.f)
	{
		FromSettings = ToSettings;
		bTransitioning = false;
		SetActorTickEnabled(false);
		ApplySettings(ToSettings, /*bRecaptureSky=*/true);
	}
	else
	{
		ApplySettings(Blended, /*bRecaptureSky=*/false);
	}
}

void ADayNightController::ApplySettings(const FLightPhaseSettings& S, bool bRecaptureSky)
{
	if (SunLight)
	{
		SunLight->SetActorRotation(S.SunRotation);
		if (ULightComponent* LC = SunLight->GetLightComponent())
		{
			LC->SetIntensity(S.SunIntensity);
			LC->SetLightColor(S.SunColor);
		}
	}

	if (SkyLight)
	{
		if (USkyLightComponent* SLC = SkyLight->GetLightComponent())
		{
			SLC->SetIntensity(S.SkyLightIntensity);
			if (bRecaptureSky && !SLC->bRealTimeCapture)
			{
				SLC->RecaptureSky();
			}
		}
	}
}

FLightPhaseSettings ADayNightController::BlendSettings(const FLightPhaseSettings& A, const FLightPhaseSettings& B, float Alpha)
{
	FLightPhaseSettings R;
	R.SunRotation = FQuat::Slerp(A.SunRotation.Quaternion(), B.SunRotation.Quaternion(), Alpha).Rotator();
	R.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
	R.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
	R.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
	return R;
}
