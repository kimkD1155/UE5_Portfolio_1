// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "Barricade.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
// 이 바리케이드가 파괴되는 순간 1회 브로드캐스트. GameMode 가 전멸 여부를 카운트한다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarricadeDestroyed, ABarricade*, Barricade);

/**
 * 체력은 이 액터가 갖지 않고 UBarricadeManager(WorldSubsystem)의 공유 풀에 위임한다 —
 * 서로 다른 메시의 바리케이드를 여러 개 배치해도 "하나의 방어선"으로 묶여 체력을 공유하기
 * 위해서다. 이 클래스는 자기 몫의 최대체력(BaseMaxHealth)을 공유 풀에 등록하고, 피해/수리를
 * 매니저로 전달하고, 매니저가 알려주는 공유 상태를 자기 시각 효과(HP 델리게이트, 파괴 연출)로
 * 중계하는 역할만 한다.
 */
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

	// 메시 애셋의 실제 바운즈에 맞춰 BlockingVolume 크기를 자동으로 맞춘다.
	// 에디터에서 메시를 바꿔도 바로 반영되고, 메시 자체에 콜리전이 없어도 확실히 막는다.
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


public:
	// IInteractableInterface
	virtual void Interact_Implementation(ACharacter* Interactor) override;
	virtual FText GetInteractHintText_Implementation() override;

	// 기존 위젯/HUD 가 구독하는 델리게이트. UBarricadeManager 의 공유 이벤트를 이 시그니처로 중계한다.
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

	// UBarricadeManager 가 공유 풀이 0이 됐을 때 등록된 바리케이드 전부에 호출한다.
	void NotifyDestroyedByManager();

protected:
	// UBarricadeManager::OnHPChanged 에 바인딩 — 공유 체력을 이 인스턴스의 OnHPChanged 로 중계한다.
	UFUNCTION()
	void HandleSharedHPChanged(float Health, float MaxHealth);

	// 좀비(AEnemyAIController)가 가한 피해만 받아들여 공유 풀로 전달한다 —
	// 플레이어/아군이 자기 바리케이드를 총으로 쏴서 깎는 걸 막는다.
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	// 파괴 시 물리/충돌 정리
	void ApplyDestroyedState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* BlockingVolume;

	// E 키로 한 번 상호작용 시 공유 풀에 회복되는 양
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float RepairAmountPerInteract = 50.f;

	// 이 바리케이드 조각이 공유 풀의 최대체력에 기여하는 양. 서로 다른 메시(크기)의 바리케이드마다
	// 다르게 줄 수 있다 — 조각을 더 놓을수록 방어선 전체 체력이 늘어난다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barricade")
	float BaseMaxHealth = 90.f;
};
