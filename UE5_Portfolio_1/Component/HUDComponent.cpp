// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UHUDComponent::UHUDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHUDComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (CrosshairWidgetClass)
	{
		CrosshairWidget = CreateWidget<UCrosshairWidget>(GetWorld(), CrosshairWidgetClass);
		if (CrosshairWidget)
			CrosshairWidget->AddToViewport();
	}

	if (InteractHintWidgetClass)
	{
		InteractHintWidget = CreateWidget<UInteractHintWidget>(GetWorld(), InteractHintWidgetClass);
		if (InteractHintWidget)
		{
			InteractHintWidget->AddToViewport();
			InteractHintWidget->HideHint();
		}
	}
	
}


// Called every frame
void UHUDComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHUDComponent::ShowInteractHint(const FText& Text)
{
	if (!InteractHintWidget) return;
	InteractHintWidget->SetHintText(Text);
	InteractHintWidget->ShowHint();
}

void UHUDComponent::HideInteractHint()
{
	if (!InteractHintWidget) return;
	InteractHintWidget->HideHint();
}