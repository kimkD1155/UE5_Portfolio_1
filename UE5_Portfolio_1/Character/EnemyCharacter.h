// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;
class UAnimMontage;
class UHealthComponent;

DECLARE_MULTICAST_DELEGATE(FOnAttackMontageEnded);


UCLASS()
class UE5_PORTFOLIO_1_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure)
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

protected:

	// 체력/사망은 HealthComponent 가 담당한다. 이 캐릭터는 결과 이벤트만 구독한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UFUNCTION()
	void HandleHealthChanged(float Health, float MaxHealth, float Delta, AActor* DamageInstigator);

	UFUNCTION()
	void HandleDeath(AActor* DamageInstigator);

	// ApplyPointDamage 로 들어온 피격을 받아 맞은 본(Hit.BoneName)에 따라 배율을 곱한 뒤
	// HealthComponent 로 전달한다. (HealthComponent 의 OnTakeAnyDamage 자동 바인딩은 꺼져 있음)
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy,
		FVector HitLocation, class UPrimitiveComponent* HitComp, FName BoneName,
		FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	// 맞은 본 이름을 키워드와 대조해 존으로 분류: "Head" / "Limb" / "Torso".
	FName ResolveHitZone(FName BoneName) const;

	// 존별 데미지 배율. "Head"/"Limb" 외에는 1.0(몸통).
	float GetZoneMultiplier(FName Zone) const;

	// PIE 디버그 드로잉 (Shipping 에서는 본문이 컴파일 제외됨)
	void DrawHitZoneDebug(FName Zone, FName BoneName, const FVector& HitLocation,
		float Multiplier, float FinalDamage) const;

	//────────────────────────────────────────────
	// 부위별 피격 (머리 / 몸통 / 팔다리)
	// 배율과 본 키워드를 모두 EditDefaultsOnly 로 노출 → 좀비 타입별 BP 에서 튜닝 가능.
	// 예: Tanker 는 HeadDamageMultiplier 를 1.5 로 낮춰 "단단한 머리" 표현.

	// 머리 피격 배율 (기본 2.0 = 헤드샷 2배)
	UPROPERTY(EditDefaultsOnly, Category = "HitZone", meta = (ClampMin = "0.0"))
	float HeadDamageMultiplier = 2.0f;

	// 팔다리 피격 배율 (기본 0.6 = 사지 피해 감소)
	UPROPERTY(EditDefaultsOnly, Category = "HitZone", meta = (ClampMin = "0.0"))
	float LimbDamageMultiplier = 0.6f;

	// 본 이름에 이 문자열이 포함되면 머리로 판정 (대소문자 무시, 부분 일치)
	UPROPERTY(EditDefaultsOnly, Category = "HitZone")
	TArray<FName> HeadBoneKeywords = { TEXT("head"), TEXT("neck") };

	// 본 이름에 이 문자열이 포함되면 팔다리로 판정 (대소문자 무시, 부분 일치)
	UPROPERTY(EditDefaultsOnly, Category = "HitZone")
	TArray<FName> LimbBoneKeywords = { TEXT("arm"), TEXT("hand"), TEXT("shoulder"),
		TEXT("leg"), TEXT("foot"), TEXT("toe"), TEXT("thigh"), TEXT("calf") };

	// PIE 디버그: 피격 시 적중 지점에 존별 색상 구체와 "존 · 배율 · 최종 데미지" 텍스트를 그린다.
	// 머리=빨강, 몸통=노랑, 팔다리=하늘. Shipping 빌드에서는 자동으로 컴파일 제외.
	UPROPERTY(EditAnywhere, Category = "HitZone|Debug")
	bool bDrawHitZoneDebug = true;

	// 디버그 표시 유지 시간(초)
	UPROPERTY(EditAnywhere, Category = "HitZone|Debug", meta = (ClampMin = "0.0"))
	float HitZoneDebugDuration = 2.f;

	virtual void Attack() {}

	void Die();

	// AActor::GetTargetLocation(AActor*) 와 이름이 겹치지 않도록 GetTargetActor 로 명명
	AActor* GetTargetActor() const;

	// 이 좀비가 한 대 때릴 때의 데미지
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackDamage = 30.f;

	// 서브클래스 생성자에서 HealthComponent 에 넣어줄 최대 체력
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MaxHealth = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 CoinReward = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float MoveSpeed = 100.f;

	// 언리얼 기본 Distance 가 최소 188 정도 되므로, 자식 Enemy 에서 그 이상으로 해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float AttackRange = 100.f;

	UPROPERTY()
	bool bIsPlayingHitReaction = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	USphereComponent* AttackHitBox;

	//────────────────────────────────────────────
	// 몽타주 및 애니메이션 관련
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DieMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	void PlayDieMontage(UAnimMontage* MontageToPlay);
	void PlayAttackMontage();
	void PlayHitReactionMontage(UAnimMontage* MontageToPlay);

	void OnDieMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnAttackMontageEnded OnAttackMontageEnded;

	// AnimNotify 에서 호출
	void EnableAttackHitBox();
	void DisableAttackHitBox();

private:
	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION()
	void OnAttackHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnAttackMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted);
};
