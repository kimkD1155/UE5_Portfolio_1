// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h" // FOnMontageEnded

class UAnimMontage;
class USkeletalMeshComponent;

/**
 * 몽타주 재생 + 종료 콜백 바인딩의 반복 패턴을 한 곳으로 모은 헬퍼.
 * 여러 캐릭터 클래스(플레이어/적/아군)가 동일하게 사용한다.
 */
namespace MontageHelper
{
	// 종료 콜백 없이 몽타주만 재생. @return 재생 길이(초), 실패 시 0.
	float Play(USkeletalMeshComponent* Mesh, UAnimMontage* Montage, float PlayRate = 1.f);

	// 실제 구현. Mesh 의 애님 인스턴스로 Montage 를 재생하고 종료 델리게이트를 건다.
	// @return 재생된 몽타주 길이(초). Mesh/Montage/AnimInstance 가 없거나 재생 실패 시 0.
	float PlayWithEndDelegate(
		USkeletalMeshComponent* Mesh,
		UAnimMontage* Montage,
		FOnMontageEnded EndDelegate,
		float PlayRate = 1.f);

	// 편의 래퍼: 멤버 함수 포인터를 받아 바인딩까지 처리하므로 호출부는 한 줄로 끝난다.
	template <typename UserClass>
	float PlayWithEndCallback(
		USkeletalMeshComponent* Mesh,
		UAnimMontage* Montage,
		UserClass* Object,
		void (UserClass::* Callback)(UAnimMontage*, bool),
		float PlayRate = 1.f)
	{
		FOnMontageEnded Delegate;
		Delegate.BindUObject(Object, Callback);
		return PlayWithEndDelegate(Mesh, Montage, MoveTemp(Delegate), PlayRate);
	}
}
