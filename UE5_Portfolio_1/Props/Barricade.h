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
// 이 바리케이드가 파괴되는 순간 1회 브로드캐스트. GameMode 가 전멸 여부를 카운트한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarricadeDestroyed, ABarricade*, Barricade);

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

	UPROPERTY(BlueprintAssignable, Category = "Barricade")
	FOnBarricadeDestroyed OnBarricadeDestroyed;

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

	// 바리케이드 체력 업그레이드 반영 — 최대 체력을 BaseMaxHealth * 배율 로 다시 설정
	UFUNCTION()
	void HandleUpgradesChanged();

	// PlayerState 가 아직 없을 수 있어 재시도한다
	void BindToPlayerState();

	// 파괴 시 물리/충돌 정리
	void ApplyDestroyedState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BlockingVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	// E 키로 한 번 상호작용 시 회복되는 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float RepairAmountPerInteract = 50.f;

	// 업그레이드 0레벨 기준 최대 체력. 실제 최대 체력 = BaseMaxHealth * (1 + Lv*배율)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float BaseMaxHealth = 90.f;

	UPROPERTY()
	TWeakObjectPtr<class AKangPlayerState> BoundPlayerState;

	FTimerHandle BindRetryHandle;
};
