// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AllyPlacementArea.generated.h"

class UBoxComponent;

UCLASS()
class UE5_PORTFOLIO_1_API AAllyPlacementArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAllyPlacementArea();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	bool IsLocationInside(const FVector& Location) const;

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComponent;
};
