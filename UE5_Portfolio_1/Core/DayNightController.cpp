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
	NightSettings.SunRotation = FRotator(20.f, -30.f, 0.f);   // 지평선 아래
	NightSettings.SunIntensity = 0.05f;
	NightSettings.SunColor = FLinearColor(0.15f, 0.2f, 0.4f);
	NightSettings.SkyLightIntensity = 0.1f;
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
