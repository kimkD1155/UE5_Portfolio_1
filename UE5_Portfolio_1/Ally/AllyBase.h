// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AllyBase.generated.h"

class AWeaponBase;
class USphereComponent;
class ARangedWeapon;

UCLASS()
class UE5_PORTFOLIO_1_API AAllyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAllyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


//적 감지 관련
protected:
	// 적 감지용 구체 콜리전 (이 범위 안에 들어온 적만 타겟팅 대상)
	UPROPERTY(EditDefaultsOnly)
	USphereComponent* AttackRangeSphere;
	// 감지 반경
	UPROPERTY(EditDefaultsOnly)
	float AttackRange{ 800.f };
	// 현재 범위 안에 들어와 있는 적 목록
	UPROPERTY()
	TArray<AActor*> EnemiesInRange;
	// 현재 공격 대상으로 선택된 적
	UPROPERTY()
	AActor* CurrentTarget;

	AActor* FindTarget(); // 지금은 가장 가까운 적만

	// 적이 AttackRangeSphere 안으로 들어왔을 때 호출
	UFUNCTION()
	void OnEnemyEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// 적이 AttackRangeSphere 밖으로 나갔을 때 호출
	UFUNCTION()
	void OnEnemyExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//전투
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;
	// 장착된 무기 (사격 시 캐스팅해서 사용)
	UPROPERTY()
	AWeaponBase* EquippedWeapon;
	void EquipDefaultWeapon();

	void Attack();

	UPROPERTY()
	FTimerHandle AttackTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackInterval = 1.f;


	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* PistolReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RifleReloadMontage;

	// 현재 리로드 중인 무기 (완료 시점에 ReloadFinished() 호출 대상)
	UPROPERTY()
	ARangedWeapon* ReloadingWeapon;

	void Reload();
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//디버그
protected:
	// 디버그 시각화 여부 (에디터에서 끄고 켤 수 있게)
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShowAttackRangeDebug = true;

};
