// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DayPhaseAnnounceWidget.generated.h"

class UTextBlock;

/**
 * Day 시작/종료를 알리는 전체화면 안내판. 불투명한 검은 배경(BP에서 구성) 위에 큰 텍스트를
 * 잠깐 띄워 게임 플레이 화면 자체를 가린다 — 다른 HUD 위젯 위에 겹쳐 보이는 방식이 아니라
 * 최상단 Z-Order로 전체를 덮어써서 "화면이 잠깐 완전히 바뀌는" 느낌을 낸다.
 */
UCLASS(Abstract)
class UE5_PORTFOLIO_1_API UDayPhaseAnnounceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Message 를 큐에 넣는다. 표시 중인 메시지가 없으면 즉시 시작하고, 있으면 그 뒤를 이어
	// Duration 초씩 순서대로 보여준다 (예: "Day 1 End" 다음 "Day 2 Start" 를 끊기지 않게 이어서).
	// 큐가 완전히 비면 자동으로 다시 숨는다. HUDComponent::HandlePhaseChanged 가 호출.
	UFUNCTION(BlueprintCallable, Category = "Announce")
	void ShowAnnouncement(const FText& Message, float Duration = 3.f);

	// 큐에 쌓인 메시지를 전부 다 보여주고 완전히 숨었을 때 브로드캐스트.
	// HUDComponent 가 이 시점에 일시정지/입력 차단을 해제한다.
	FSimpleMulticastDelegate OnQueueFinished;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void PlayNextInQueue();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnounceText;

	FTimerHandle HideTimerHandle;

	TArray<TPair<FText, float>> PendingQueue;

	// PendingQueue.Num()==0 은 "지금 막 하나를 꺼내 보여주는 중"과 "완전히 다 끝남"을 구분하지
	// 못한다 — 같은 프레임에 ShowAnnouncement를 연달아 호출하면(End 다음 Start) 두 번째 호출이
	// 첫 번째가 아직 표시 중인데도 "유휴 상태"로 오판해 타이머를 덮어써버리는 버그가 있었다.
	bool bIsShowingAnnouncement = false;
};
