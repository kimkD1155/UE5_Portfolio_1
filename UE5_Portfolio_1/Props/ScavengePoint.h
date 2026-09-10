// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "../Core/GamePhase.h"
#include "ScavengePoint.generated.h"

class UStaticMeshComponent;

/**
 * 낮(수색) 국면에만 상호작용 가능한 물자 더미.
 * 상호작용하면 코인을 주고, 기본적으로 하루 1회만 사용 가능하다.
 * (AShop / ABarricade 와 동일한 IInteractableInterface 패턴.)
 */
UCLASS()
class UE5_PORTFOLIO_1_API AScavengePoint : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AScavengePoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Scavenge", meta = (ClampMin = "0"))
	int32 CoinReward = 40;

	// true = 낮마다 1회. false = 낮 동안 무제한.
	UPROPERTY(EditAnywhere, Category = "Scavenge")
	bool bOncePerDay = true;

	bool bUsedThisDay = false;

	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	void SetAvailable(bool bAvailable);

	bool CanScavengeNow() const;

public:
	// IInteractableInterface
	virtual void Interact_Implementation(ACharacter* Interactor) override;
	virtual FText GetInteractHintText_Implementation() override;
};
