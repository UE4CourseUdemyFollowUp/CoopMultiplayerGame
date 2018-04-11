// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
	: PowerupInterval(0.f)
	, TotalNrOfTicks(0)
	, TicksProcessed(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASPowerupActor::OnPowerupTick()
{
	++TicksProcessed;

	OnTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		GetWorldTimerManager().ClearTimer(TimerHanlde_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHanlde_PowerupTick, this, &ASPowerupActor::OnPowerupTick, PowerupInterval, true);
	}
	else
	{
		OnPowerupTick();
	}
}
