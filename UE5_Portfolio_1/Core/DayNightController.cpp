// Fill out your copyright notice in the Description page of Project Settings.


#include "DayNightController.h"
#include "KangGameState.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"

ADayNightController::ADayNightController()
{
	// Actor Tick 을 쓰지 않는다 — 레벨에 배치된 인스턴스가 Tick 이 꺼진 채로 저장돼 있으면
	// (BP 클래스 디폴트 오버라이드 등) 전환 로직이 통째로 멈춰버리는데, 이게 바로 이 프로젝트에서
	// 실제로 겪은 버그다. Heartbeat() 를 타이머로 직접 돌리면 Actor Tick 설정과 완전히
	// 무관하게 항상 동작한다.
	PrimaryActorTick.bCanEverTick = false;

	// 낮 기본값은 struct 기본값을 그대로 쓰고, 밤 기본값만 밤답게 덮어쓴다.
	//
	// SunRotation 은 낮과 완전히 동일하게 고정한다 — SkyAtmosphere는 라이트의 밝기/색이 아니라
	// 지평선 대비 각도만 보고 낮/밤을 판정하므로, 각도를 조금이라도 바꾸면 하늘 자체의 밝기가
	// (원하든 원치 않든) 같이 흔들린다. 태양을 아예 안 움직이면 하늘 밝기는 낮/밤 내내 동일하게
	// 유지되고, "밤 같은 느낌"은 전적으로 아래 색/밝기 값으로만 만든다 — 사실적인 어두운 밤하늘은
	// 포기하고, 대신 캐릭터/지형이 충분히 밝게 보이는 스타일화된 밤을 선택한 것.
	NightSettings.SunRotation = DaySettings.SunRotation;
	NightSettings.SunIntensity = 0.8f;                          // 낮(3.0)보다 확 낮춰 밤임을 밝기 대비로 전달
	NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.85f);   // 차가운 달빛 톤 — 하늘 밝기가 안 바뀌는 만큼 색 대비를 더 뚜렷하게
	NightSettings.SkyLightIntensity = 0.5f;                     // 태양 각도가 고정이라 주변광도 낮보다 낮춰야 밤 대비가 산다
	NightSettings.bShowSunDisc = false;                         // 태양 위치가 고정이라, 밤에도 하늘에 해가 떠있으면 어색하니 원반만 숨김
}

void ADayNightController::BeginPlay()
{
	Super::BeginPlay();

	// 이 줄이 Output Log에 안 찍히면 이 액터의 BeginPlay 자체가 실행되지 않는 것 —
	// 레벨에 없거나, BP 오버라이드에서 Parent: BeginPlay 호출이 빠졌거나, 로드 안 된
	// 서브레벨에 있다는 뜻. (화면 오버레이와 달리 이건 항상 로그에 남는다.)
	UE_LOG(LogTemp, Warning, TEXT("[DayNight] BeginPlay called on %s in world %s"),
		*GetName(), *GetWorld()->GetMapName());

	if (!SunLight)
	{
		SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(this, ADirectionalLight::StaticClass()));
	}
	if (!SkyLight)
	{
		SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(this, ASkyLight::StaticClass()));
	}

	TryBindGameState();

	GetWorldTimerManager().SetTimer(HeartbeatTimerHandle, this, &ADayNightController::Heartbeat, HeartbeatInterval, true);
}

void ADayNightController::TryBindGameState()
{
	if (bBoundToGameState) return;

	AKangGameState* GS = GetWorld() ? GetWorld()->GetGameState<AKangGameState>() : nullptr;
	UE_LOG(LogTemp, Warning, TEXT("[DayNight] TryBindGameState: GameState valid = %s"), GS ? TEXT("YES") : TEXT("NO"));
	if (!GS) return; // 아직 GameState 가 준비 안 됐으면 다음 Heartbeat에서 다시 시도한다.

	bBoundToGameState = true;
	GS->OnPhaseChanged.AddDynamic(this, &ADayNightController::HandlePhaseChanged);

	// 시작 국면을 즉시(스냅) 반영
	const EGamePhase Phase = GS->GetCurrentPhase();
	const FLightPhaseSettings& Initial = (Phase == EGamePhase::Night) ? NightSettings : DaySettings;
	FromSettings = ToSettings = Initial;
	ApplySettings(Initial, /*bRecaptureSky=*/true);
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
		return;
	}

	// 현재 적용된 값(FromSettings)에서 Target 으로 보간 시작
	TransitionElapsed = 0.f;
	bTransitioning = true;
}

void ADayNightController::Heartbeat()
{
	if (!bBoundToGameState)
	{
		TryBindGameState();
	}

	if (!bTransitioning)
	{
		// 전환 중이 아니어도 목표값을 계속 재적용한다 (다른 시스템과의 충돌 방지 — 위 생성자 주석 참고).
		ApplySettings(ToSettings, /*bRecaptureSky=*/false);
		return;
	}

	TransitionElapsed += HeartbeatInterval;
	const float Alpha = FMath::Clamp(TransitionElapsed / FMath::Max(TransitionTime, KINDA_SMALL_NUMBER), 0.f, 1.f);

	const FLightPhaseSettings Blended = BlendSettings(FromSettings, ToSettings, Alpha);

	if (Alpha >= 1.f)
	{
		FromSettings = ToSettings;
		bTransitioning = false;
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
			if (UDirectionalLightComponent* DLC = Cast<UDirectionalLightComponent>(LC))
			{
				// SkyAtmosphere가 그리는 하늘의 태양 원반 표시 여부. 태양 위치를 고정해두는
				// 설계라, 이걸로 밤에는 원반만 숨겨서 "해가 떠있는데 밤"처럼 안 보이게 한다.
				DLC->SetAtmosphereSunLight(S.bShowSunDisc);
			}
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

	// bRecaptureSky=true 는 전환 완료/초기 스냅 시점에만 한 번씩 오니, 여기서만 로그를 남겨도
	// 매 프레임 스팸 없이 "우리가 실제로 뭘 어떻게 세팅했는지" 를 Output Log 로 확인할 수 있다.
	if (bRecaptureSky)
	{
		const FRotator ReadBackRot = SunLight ? SunLight->GetActorRotation() : FRotator::ZeroRotator;
		const float ReadBackIntensity = (SunLight && SunLight->GetLightComponent()) ? SunLight->GetLightComponent()->Intensity : -1.f;
		UE_LOG(LogTemp, Warning, TEXT("[DayNight] ApplySettings: SunLight=%s TargetRot=%s ReadBackRot=%s TargetIntensity=%.2f ReadBackIntensity=%.2f"),
			SunLight ? *SunLight->GetName() : TEXT("NULL"),
			*S.SunRotation.ToCompactString(), *ReadBackRot.ToCompactString(),
			S.SunIntensity, ReadBackIntensity);
	}

#if ENABLE_DRAW_DEBUG
	// 임시 진단용: 이 함수가 실제로 뭘 하고 있는지(대상이 유효한지, 무슨 값을 쓰는지) 화면에 그대로 노출한다.
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(9500, 0.f, FColor::Emerald, FString::Printf(
			TEXT("[DayNight] SunLight=%s Rot=%s Intensity=%.2f | SkyLight=%s Intensity=%.2f | Transitioning=%s"),
			SunLight ? *SunLight->GetName() : TEXT("NULL"),
			*S.SunRotation.ToCompactString(), S.SunIntensity,
			SkyLight ? *SkyLight->GetName() : TEXT("NULL"), S.SkyLightIntensity,
			bTransitioning ? TEXT("Y") : TEXT("N")));
	}*/
#endif
}

FLightPhaseSettings ADayNightController::BlendSettings(const FLightPhaseSettings& A, const FLightPhaseSettings& B, float Alpha)
{
	FLightPhaseSettings R;
	R.SunRotation = FQuat::Slerp(A.SunRotation.Quaternion(), B.SunRotation.Quaternion(), Alpha).Rotator();
	R.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
	R.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
	R.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
	R.bShowSunDisc = B.bShowSunDisc; // 불리언은 페이드가 안 되니, 전환 시작과 동시에 목표 값으로 바로 반영
	return R;
}
