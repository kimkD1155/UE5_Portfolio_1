// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "Barricade.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

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

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsDestroyed() const { return CurrentHealth <= 0.f; }

	UFUNCTION(BlueprintPure, Category = "Barricade")
	bool IsFullHealth() const { return CurrentHealth >= MaxHealth; }

protected:
	/*UFUNCTION()
	virtual float TakeDamageHandler(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);*/

	void Repair(float RepairAmount);
	void OnBarricadeDestroyed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BlockingVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float MaxHealth = 500.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade")
	float CurrentHealth;

	// E키 한 번에 회복되는 양 (못질 한 번 분량)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float RepairAmountPerInteract = 50.f;
};
