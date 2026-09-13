// Fill out your copyright notice in the Description page of Project Settings.


#include "KangPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void AKangPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC, 0);
	}

	// 메인 메뉴 등 이전 레벨에서 UI 전용 입력 모드로 바꿔놨을 수 있다.
	// Input Mode 는 LocalPlayer/뷰포트에 걸리는 상태라 Open Level 로 넘어와도 안 풀리므로,
	// 게임플레이 레벨은 시작하자마자 스스로 게임 전용 입력을 다시 선언한다.
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}