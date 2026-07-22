// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyPlacementArea.h"
#include "Components/BoxComponent.h"

// Sets default values
AAllyPlacementArea::AAllyPlacementArea()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}

// Called when the game starts or when spawned
void AAllyPlacementArea::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAllyPlacementArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AAllyPlacementArea::IsLocationInside(const FVector& Location) const
{
	FVector BoxOrigin = BoxComponent->GetComponentLocation();
	FVector BoxExtent = BoxComponent->GetScaledBoxExtent();
	FBox Box(BoxOrigin - BoxExtent, BoxOrigin + BoxExtent);
	return Box.IsInsideOrOn(Location);
}