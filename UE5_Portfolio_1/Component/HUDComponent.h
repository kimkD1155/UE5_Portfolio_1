// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Widget/InteractHintWidget.h"
#include "../Widget/CrosshairWidget.h"
#include "HUDComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_PORTFOLIO_1_API UHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHUDComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
public:
	void ShowInteractHint(const FText& Text);
	void HideInteractHint();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UInteractHintWidget> InteractHintWidgetClass;

private:
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	UCrosshairWidget* CrosshairWidget = nullptr;

	UPROPERTY()
	UInteractHintWidget* InteractHintWidget = nullptr;
};
