// Fill out your copyright notice in the Description page of Project Settings.

#include "SPickupActor.h"
#include "PowerUps/SPowerupActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"


// Sets default values
ASPickupActor::ASPickupActor()
	: PowerupActorInstance(nullptr)
	, CooldownDuration(10.f)
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	DecalComp->DecalSize = FVector(65.f, 75.f, 75.f);
	DecalComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASPickupActor::BeginPlay()
{
	Super::BeginPlay();
	
	Respawn();
}

void ASPickupActor::Respawn()
{
	if (PowerupActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerupActorClass isn't set for &s. Please update your blueprint"), *GetName());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PowerupActorInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerupActorClass, GetTransform(), SpawnParams);
}

void ASPickupActor::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (PowerupActorInstance)
	{
		PowerupActorInstance->ActivatePowerup();
		PowerupActorInstance = nullptr;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickupActor::Respawn, CooldownDuration);
	}
}