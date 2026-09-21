// Fill out your copyright notice in the Description page of Project Settings.

#include "DayPhaseAnnounceWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UDayPhaseAnnounceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// BP 클래스 디폴트가 이걸 Visible 로 덮어쓸 수 있으니(이 프로젝트에서 여러 번 겪은 패턴)
	// 시작 시 확실히 숨겨둔다 — ShowAnnouncement() 가 호출될 때만 보여야 한다.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDayPhaseAnnounceWidget::ShowAnnouncement(const FText& Message, float Duration)
{
	PendingQueue.Add(TPair<FText, float>(Message, Duration));

	// 이미 뭔가 표시 중이면 큐에만 쌓아두고, 그 타이머가 끝날 때 PlayNextInQueue() 가
	// 알아서 다음 걸 이어서 보여준다.
	if (!bIsShowingAnnouncement)
	{
		PlayNextInQueue();
	}
}

void UDayPhaseAnnounceWidget::PlayNextInQueue()
{
	if (PendingQueue.Num() == 0)
	{
		bIsShowingAnnouncement = false;
		SetVisibility(ESlateVisibility::Collapsed);
		OnQueueFinished.Broadcast();
		return;
	}

	bIsShowingAnnouncement = true;
	const TPair<FText, float> Next = PendingQueue[0];
	PendingQueue.RemoveAt(0);

	if (AnnounceText)
	{
		AnnounceText->SetText(Next.Key);
	}
	SetVisibility(ESlateVisibility::Visible);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HideTimerHandle, this,
			&UDayPhaseAnnounceWidget::PlayNextInQueue, Next.Value, false);
	}
}
