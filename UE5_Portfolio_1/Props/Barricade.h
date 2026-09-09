// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "Barricade.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);

UCLASS()
class UE5_PORTFOLIO_1_API ABarricade : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABarricade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	// IInteractableInterface
	virtual void Interact_Implementation(ACharacter* Interactor) override;
	virtual FText GetInteractHintText_Implementation() override;

	// 기존 위젯/HUD 가 구독하는 델리게이트. HealthComponent 이벤트를 이 시그니처로 중계한다.
	UPROPERTY(BlueprintAssignable, Category = "Barricade")
	FOnHPChanged OnHPChanged;

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsDestroyed() const;

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsFullHealth() const;

protected:
	UFUNCTION()
	void HandleHealthChanged(float Health, float MaxHealth, float Delta, AActor* DamageInstigator);

	UFUNCTION()
	void HandleDeath(AActor* DamageInstigator);

	void OnBarricadeDestroyed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BlockingVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	// E 키로 한 번 상호작용 시 회복되는 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float RepairAmountPerInteract = 50.f;
};
