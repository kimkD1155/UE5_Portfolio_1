// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ── 커스텀 트레이스 채널 ─────────────────────────────────────────────
// DefaultEngine.ini 의 [/Script/Engine.CollisionProfile] GameTraceChannel1 정의와 대응.
// 히트스캔 무기 전용 채널: 적 스켈레탈 메시는 Block(본 단위), Pawn 캡슐은 Ignore 로 두어
// 트레이스가 캡슐을 통과해 메시에 맞는다 → Hit.BoneName 으로 머리/몸통/팔다리 판정 가능.
#define ECC_Weapon ECC_GameTraceChannel1

