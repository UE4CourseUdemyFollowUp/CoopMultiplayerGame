// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
	: PowerupInterval(0.f)
	, TotalNrOfTicks(0)
	, TicksProcessed(0)
	, bIsPowerupActive(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetReplicates(true);
}

void ASPowerupActor::OnPowerupTick()
{
	++TicksProcessed;

	OnTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		GetWorldTimerManager().ClearTimer(TimerHanlde_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChange(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHanlde_PowerupTick, this, &ASPowerupActor::OnPowerupTick, PowerupInterval, true);
	}
	else
	{
		OnPowerupTick();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}
