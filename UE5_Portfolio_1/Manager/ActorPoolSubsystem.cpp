// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorPoolSubsystem.h"
#include "../Interface/Poolable.h"

AActor* UActorPoolSubsystem::Acquire(TSubclassOf<AActor> Class, const FVector& Location, const FRotator& Rotation)
{
	if (!Class) return nullptr;

	AActor* Instance = nullptr;

	if (FPooledActorList* Bucket = Pools.Find(Class))
	{
		while (Bucket->Actors.Num() > 0 && !Instance)
		{
			AActor* Candidate = Bucket->Actors.Pop();
			if (IsValid(Candidate))
			{
				Instance = Candidate;
			}
		}
	}

	if (Instance)
	{
		Instance->SetActorLocationAndRotation(Location, Rotation);
		Instance->SetActorHiddenInGame(false);
		Instance->SetActorEnableCollision(true);
		Instance->SetActorTickEnabled(true);
		UE_LOG(LogTemp, Verbose, TEXT("[ActorPool] 재사용: %s"), *Class->GetName());
	}
	else
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		Instance = GetWorld()->SpawnActor<AActor>(Class, Location, Rotation, Params);
		UE_LOG(LogTemp, Verbose, TEXT("[ActorPool] 신규 스폰: %s"), *Class->GetName());
	}

	if (IPoolable* Poolable = Cast<IPoolable>(Instance))
	{
		Poolable->OnAcquiredFromPool();
	}

	return Instance;
}

void UActorPoolSubsystem::Release(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	if (IPoolable* Poolable = Cast<IPoolable>(Actor))
	{
		Poolable->OnReturnedToPool();
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	Pools.FindOrAdd(Actor->GetClass()).Actors.Add(Actor);
}
