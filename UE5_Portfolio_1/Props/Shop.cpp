// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "../Widget/ShopWidget.h"

// Sets default values
AShop::AShop()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

// Called every frame
void AShop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned
void AShop::BeginPlay()
{
	Super::BeginPlay();

	if (ShopWidgetClass)
	{
		ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass);
		if (ShopWidget)
		{
			ShopWidget->InitShop(this);
			ShopWidget->AddToViewport();
			ShopWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}



void AShop::Interact_Implementation(ACharacter* Interactor)
{
	if (!ShopWidget) return;

	bool bIsVisible = ShopWidget->GetVisibility() == ESlateVisibility::Visible;
	ShopWidget->SetVisibility(bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	bIsShopOpen = !bIsVisible;

	APlayerController* PC = Cast<APlayerController>(Interactor->GetController());
	if (!PC) return;

	if (!bIsVisible)
	{
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

FText AShop::GetInteractHintText_Implementation()
{
	return FText::FromString(TEXT("Open Shop [E]"));
}

void AShop::CloseShop()
{
	if (!ShopWidget || !bIsShopOpen) return;
	ShopWidget->SetVisibility(ESlateVisibility::Hidden);
	bIsShopOpen = false;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}